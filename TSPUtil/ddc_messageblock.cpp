
#include "ddc_util.h"
#include "ddc_debuglog.h"
#include "ddc_memory.h"
#include "ddc_messageblock.h"


namespace nm_ddc{

CMessageBlock::CMessageBlock(CDdcAllocate *message_block_allocator)
		:base_(0),
		cur_size_(0),
		max_size_(0),
		rd_ptr_(0),
		wr_ptr_(0),
		allocator_strategy_(message_block_allocator),
		type_(CMessageBlock::MB_DATA),
		flags_(0),
		reference_count_(1)
{}

CMessageBlock::CMessageBlock( const char *buff,size_t buflen,int type,CDdcAllocate *message_block_allocator )
		:base_(const_cast<char *>(buff)),
		cur_size_(buflen),
		max_size_(buflen),
		rd_ptr_(0),
		wr_ptr_(0),
		allocator_strategy_(message_block_allocator),
		type_(type),
		flags_(CMessageBlock::DONT_DELETE),
		reference_count_(1)
{}

CMessageBlock::CMessageBlock (size_t buflen,int type ,const char *data ,CDdcAllocate *message_block_allocator )
		:cur_size_(buflen),
		max_size_(buflen),
		rd_ptr_(0),
		wr_ptr_(0),
		allocator_strategy_(message_block_allocator),
		type_(type),
		flags_(0),
		reference_count_(1)
{
	base_ = (char *) this->allocator_strategy_->malloc (buflen);
}

CMessageBlock::CMessageBlock( const CMessageBlock &)
{

}


int	CMessageBlock::copy (const char *buf, size_t n)
{
	DDC_LOG(("Trace","CMessageBlock::copy\n"));
	
	// Note that for this to work correct, end () *must* be >= mark ().
	size_t len = this->space ();

	if (len < n)
		return -1;
	else
	{
		memcpy ((void *)this->wr_ptr (),buf,n);
		this->wr_ptr (n);
		return 0;
	}
}

int CMessageBlock::copy (const char *buf)
{
	return this->copy(buf,strlen (buf) + 1);
}

int CMessageBlock::crunch (void)
{
	if (this->rd_ptr_ != 0)
	{
		if (this->rd_ptr_ > this->wr_ptr_)
			return -1;

		size_t len = this->length ();
		memmove (( void *)this->base (),	this->rd_ptr (),len);
		this->rd_ptr (this->base ());
		this->wr_ptr (this->base () + len);
	}
	return 0;
}

void CMessageBlock::dump (void) const
{
#if defined (_DDC_DEBUG)
	DDC_LOG(("Dump","CMessageBlock::dump\n"));	
	DDC_LOG(("Dump","data:%s,rd_ptr(%d),wr_ptr(%d),cur_size(%d),max_size(%d)\n",rd_ptr_,wr_ptr_,cur_size_,max_size_));
#endif _DDC_DEBUG
}

int CMessageBlock::size (size_t length)
{
	DDC_LOG (("Trace","CMessageBlock::size\n"));

	if (length <= this->max_size_)
		this->cur_size_ = length;
	else
	{
		// We need to resize!
		char *buf = 0;
		buf = (char *) this->allocator_strategy_->malloc (length);
		if( !buf )
			return -1;

		memcpy (buf,this->base_,this->cur_size_);
		if (DDC_BIT_DISABLED (this->flags_,	CMessageBlock::DONT_DELETE))
			this->allocator_strategy_->free ((void *) this->base_);
		else
			// We now assume ownership.
			DDC_CLR_BITS (this->flags_,	CMessageBlock::DONT_DELETE);

		this->max_size_ = length;
		this->cur_size_ = length;
		this->base_ = buf;
	}
	return 0;
}

int CMessageBlock::release (void)
{
	DDC_LOG (("Trace","CMessageBlock::release\n"));	

	if( reference_count_ == 0 )
		return 0;
	--reference_count_;

	if ( DDC_BIT_DISABLED(this->flags_,CMessageBlock::DONT_DELETE) && this->base_ )
	{
		this->allocator_strategy_->free(this->base_);
		this->base_ = 0;
	}
	delete this;
	return 0;
}

CMessageBlock::~CMessageBlock ()
{
	DDC_LOG (("Trace","CMessageBlock::~CMessageBlock\n"));		

	if( reference_count_ == 0 )
		return;
	--reference_count_;

	if (DDC_BIT_DISABLED (this->flags_,CMessageBlock::DONT_DELETE)&&this->base_)
		this->allocator_strategy_->free(this->base_);
}

}// nm_ddc


