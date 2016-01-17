#if !defined(__DDC_MESSAGE_BLOCK_H__)
#define __DDC_MESSAGE_BLOCK_H__
#include "ddc_util.h"

namespace nm_ddc
{

class CDdcAllocate;

/**
* @class CMessageBlock
* @brief Stores messages for use throughout util(particularly in an Message_Queue).
*/

class CMessageBlock
{
	/// test class
	friend class ddc_messageblock;
public:
	enum
	{
		// = Data and proto
		MB_DATA     = 0x01,		/// Undifferentiated data message
		MB_PROTO    = 0x02,		/// Undifferentiated protocol control
		// = Control messages		
		MB_BREAK    = 0x03,		/// Line break (regular and priority)
		MB_HANGUP   = 0x89,		/// Line disconnect		
		MB_ERROR    = 0x8a,		/// Fatal error used to set u.u_error
		MB_USER     = 0x200		/// User-defined message mask
	};

	enum
	{		
		DONT_DELETE = 01,		/// Don't delete the data on exit since we don't own it.
		USER_FLAGS = 0x1000		/// user defined flags start here
	};

	/**
	* @brief 缺省构造函数
	*/
	CMessageBlock(CDdcAllocate *message_block_allocator = CDdcAllocate::instance());		
	~CMessageBlock();		
	/**
	* @brief 构造函数，传入被附加的串指针和长度
	* @param data 串指针
	* @param size 串长度
	* @param rd_ptr 读指针偏移，是偏移量而不是指针
	* @param wr_ptr 写指针偏移，是偏移量而不是指针
	*/
	CMessageBlock( const char *buff,size_t buflen,int type=MB_DATA,CDdcAllocate *message_block_allocator = CDdcAllocate::instance() );
	CMessageBlock (size_t size,int type = MB_DATA,const char *data = 0,CDdcAllocate *message_block_allocator = CDdcAllocate::instance());
	CMessageBlock ( const CMessageBlock &);

	void reset()
	{
		rd_ptr_ = 0;
		wr_ptr_ = 0;
	}

	/// Set message data	
	int init(const char *buff)
	{
		return init(buff,strlen(buff));
	}

	/**
	* @brief Set message data
	* @param buff 被附加的串
	* @param size 串长度
	* @param rd_ptr 读指针偏移
	* @param wr_ptr 写指针偏移
	*/
	int init(const char *buff,size_t buflen,size_t rd_pt = 0,size_t wr_pt = 0 )
	{	

		DDC_SET_BITS(this->flags_,CMessageBlock::DONT_DELETE);

		base_ = const_cast<char *>(buff);
		cur_size_ = max_size_ = buflen;
		rd_ptr_ = rd_pt;	
		wr_ptr_ = wr_pt;
		return 0;
	}

	/// set/get mesage type
	int msg_type(void) const { return type_;};
	void msg_type(int type) { type_ = type; };

	/// set/clear/get flags
	int set_flags (int more_flags) { int oldflag=flags_; DDC_SET_BITS(flags_,more_flags);return oldflag;};
	int clr_flags (int less_flags){ int oldflag=flags_; DDC_CLR_BITS(flags_,less_flags);return oldflag;};
	int flags (void) const { return flags_;};


	/// Return an exact "deep copy" of the message, i.e., create fresh
	/// new copies of all the Data_Blocks and continuations.
	CMessageBlock *clone (void) const;

	/// Return a "shallow" copy that increments our reference count by 1.
	CMessageBlock *duplicate (void) const;

	int release(void);

	/**
   * 拷贝，从wr_ptr()开始写@arg n个字节，同时wr_ptr也移动到拷贝的字节后
   *
   * @param buf  Pointer to the buffer to copy from.
   * @param n    The number of bytes to copy.
   *
   * @retval 0  on success; the write pointer is advanced by @arg n.
   * @retval -1 if the amount of free space following the write pointer
   *            in the block is less than @arg n. Free space can be checked
   *            by calling space().
   */
	int copy (const char *buf, size_t n);
	int copy (const char *buf);



	/// Get the read pointer.
	char *rd_ptr(void) const { return base_ + rd_ptr_;};

	/// Set the read pointer to <ptr>.
	void rd_ptr(const char *ptr)
	{
		if( ptr >= base_ &&  static_cast<size_t>(ptr-base_)<cur_size_ )
			rd_ptr_ = ptr-base_;
		else
			rd_ptr_ = cur_size_;
	}

	/// Set the read pointer ahead<n> bytes.
	void rd_ptr(size_t n)
	{ 
		rd_ptr_ += n;
		if( rd_ptr_ > cur_size_)
			rd_ptr_ = cur_size_;
	}


	/// Get the write pointer.
	char *wr_ptr(void) const { return base_+wr_ptr_;};

	/// Set the write pointer to <ptr>.
	void wr_ptr(const char *ptr)
	{
		if( ptr >= base_ && static_cast<size_t>(ptr-base_)<cur_size_ )
			wr_ptr_ = ptr-base_;
		else
			wr_ptr_ = cur_size_;
	}


	/// Set write pointer ahead<n> bytes.
	void wr_ptr(size_t n) 
	{ 
		wr_ptr_ += n;
		if( wr_ptr_ > cur_size_)
			wr_ptr_ = cur_size_;
	}

	int crunch (void);

	/// Get the length of the message
	size_t length(void) const { return wr_ptr_ - rd_ptr_;};
	size_t capacity (void) const{ return max_size_ - cur_size_;};
	size_t space(void) const { return cur_size_ - wr_ptr_;};
	size_t size (void) const { return cur_size_;};	
	
	
	/// Get mssage data
	char *base(void) const { return base_;};
	char *end(void) const { return base_+cur_size_;};

	/// Get the total amount of allotted space in the message.  The amount of
	/// allotted space may be less than allocated space.
	int size(size_t length);

	void dump(void) const;	

protected:
	CDdcAllocate *allocator_strategy_;
	size_t rd_ptr_;
	size_t wr_ptr_;
	int type_;
	int flags_;
	char *base_;	
	size_t cur_size_;
	size_t max_size_;
	int reference_count_;
};




}// nm_ddc

#endif	//__DDC_MESSAGE_BLOCK_H__
