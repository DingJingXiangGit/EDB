//
// rm.h
//
// Record Manager component interface
//
// This file does not include the interface for the RID class.  This is
// found in rm_rid.h
//

#ifndef RM_H
#define RM_H

// Please DO NOT include any files other than redbase.h and pf.h in this
// file.  When you submit your code, the test program will be compiled
// with your rm.h and your redbase.h, along with the standard pf.h that
// was given to you.  Your rm.h, your redbase.h, and the standard pf.h
// should therefore be self-contained (i.e., should not depend upon
// declarations in any other file).

// Do not change the following includes
#include "redbase.h"
#include "rm_rid.h"
#include "pf.h"

#define NO_MORE_FREE_PAGE -1

//
// RM_Record: RM Record interface. The RM Record object stores the record
// RID and copies of its contents. (The copies are kept in an array of
// chars that RM_Record mallocs)
//
class RM_Record {
    static const int INVALID_RECORD_SIZE = -1;
    friend class RM_FileHandle;
public:
    RM_Record ();
    ~RM_Record();
    RM_Record& operator= (const RM_Record &record);

    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const;

    // Return the RID associated with the record
    RC GetRid (RID &rid) const;

    // Sets the record with an RID, data contents, and its size
    RC SetRecord (RID rec_rid, char *recData, int size);
private:
    RID rid;        // record RID
    char *data;     // pointer to record data. This is stored in the
                    // record object, where its size is malloced
    int size;       // size of the malloc
};

// RM_FileHeader: Header for each file
struct RM_FileHdr {
public:
    inline int GetRecordSize() { return record_size_; }
  
    inline void SetRecordSize(int recordsize) {
        record_size_ = recordsize;
    }
  
    inline int GetRecordPerPage() { return page_records_; }
  
    inline void SetRecordPerPage(int pagerecords) {
        page_records_ = pagerecords;
    }
  
    inline int GetTotalPages() { return total_pages_; }

    inline void IncTotalPages() { ++total_pages_; }
  
    inline void DecTotalPages() { --total_pages_; }
  
    inline void SetTotalPages(int totalpages) {
        total_pages_ = totalpages;
    }

    inline int GetBitmapSize() { return bitmap_size_; }
  
    inline void SetBitmapSize(int bitmapsize) {
        bitmap_size_ = bitmapsize;
    }
  
    inline int GetValidPage() { return valid_page_; }

    inline void SetValidPage(int validpage) {
        valid_page_ = validpage;
    }

private:
    int record_size_;        // record size in file
    int page_records_;   // number of records that each page can have
    int total_pages_;        // total number of pages in this file
    int bitmap_size_;        // size of the bitmap table in byte
    int valid_page_;         // the page that is available for writing data
};

//
// RM_FileHandle: RM File interface
//
class RM_FileHandle {
    static const PageNum NO_FREE_PAGES = -1;
    friend class RM_Manager;
    friend class RM_FileScan;
public:
    RM_FileHandle ();
    ~RM_FileHandle();
    RM_FileHandle& operator= (const RM_FileHandle &fileHandle);

    // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const;

    RC InsertRec  (const char *pData, RID &rid);  // Insert a new record

    RC DeleteRec  (const RID &rid);               // Delete a record

    RC UpdateRec  (const RM_Record &rec);         // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES);

    RC Initialize(PF_FileHandle &fh);

    inline bool IsOpen() { return m_is_opened; }

    int GetRecordSize() { return m_file_hdr.GetRecordSize(); }

    RC GetNextRecord(PageNum page,
                     SlotNum slot,
                     RM_Record &rec,
                     PF_PageHandle &ph,
                     bool nextPage);
    
    static int CalRecordsPerPage(int recordSize);

    static int CalBitmapSizeInByte(int recordsPerPage);

   
private:
    bool IsPageEmpty(char * bitmap);

    RC GetFirstZeroBitSlot(char * bitmap, SlotNum &slotNum);

    RC SetBit(char *bitmap, int slotNum);

    RC ResetBit(char *bitmap, int slotNum);

    RC AllocateNewPage(PF_PageHandle &ph);

    RC CheckBitSet(char* bitmap, SlotNum slotNum, bool &hasValidRecord);

    RC GetPageNumAndSlotNum(const RID &rid, PageNum pageNum, SlotNum slotNum);

    RC GetRecordsAndBitmap(const PF_PageHandle& ph,
                           char *&bitmap,
                           char *&records);

private:
    PF_FileHandle m_pf_filehandle;
    struct RM_FileHdr m_file_hdr;
    bool m_is_opened;
};

//
// RM_FileScan: condition-based scan of records in the file
//
#define BEGIN_SCAN  -1 //default slot number before scan begins
class RM_FileScan {
public:
    RM_FileScan  ();
    ~RM_FileScan ();

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan ();                             // Close the scan

private:
    RM_FileHandle *file_handle_;
    AttrType attribute_type_;
    int attribute_length_;
    int attribute_offset_;
    bool (*compare_op_) (void*, void*, int);
    void *compare_val_;
    ClientHint hint_;
    bool opened_;
    bool scan_end_;

};

//
// RM_Manager: provides RM file management
//
class RM_Manager {
public:
    RM_Manager    (PF_Manager &pfm);
    ~RM_Manager   ();

    RC CreateFile (const char *fileName, int recordSize);
    RC DestroyFile(const char *fileName);
    RC OpenFile   (const char *fileName, RM_FileHandle &fileHandle);

    RC CloseFile  (RM_FileHandle &fileHandle);
private:
    // helper method for open scan which sets up private variables of
    
    RC SetUpFH(RM_FileHandle& fileHandle, PF_FileHandle &fh,
               struct RM_FileHeader* header);
    // helper method for close scan which sets up private variables of
    // RM_FileHandle
    RC CleanUpFH(RM_FileHandle &fileHandle);

    PF_Manager &pfm; // reference to program's PF_Manager
};

//
// Print-error function
//
void RM_PrintError(RC rc);

#define RM_INVALIDRID           (START_RM_WARN + 0) // invalid RID
#define RM_BADRECORDSIZE        (START_RM_WARN + 1) // record size is invalid
#define RM_INVALIDRECORD        (START_RM_WARN + 2) // invalid record
#define RM_INVALIDBITOPERATION  (START_RM_WARN + 3) // invalid page header bit ops
#define RM_PAGEFULL             (START_RM_WARN + 4) // no more free slots on page
#define RM_INVALIDFILE          (START_RM_WARN + 5) // file is corrupt/not there
#define RM_INVALIDFILEHANDLE    (START_RM_WARN + 6) // filehandle is improperly set up
#define RM_INVALIDSCAN          (START_RM_WARN + 7) // scan is improperly set up
#define RM_ENDOFPAGE            (START_RM_WARN + 8) // end of a page
#define RM_EOF                  (START_RM_WARN + 9) // end of file 
#define RM_BADFILENAME          (START_RM_WARN + 10)
#define RM_LASTWARN             RM_BADFILENAME

#define RM_ERROR                (START_RM_ERR - 0) // error
#define RM_LASTERROR            RM_ERROR



#endif
