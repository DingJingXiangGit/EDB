#include "rm.h"
#include <math.h>

RM_FileHandle::RM_FileHandle() {
    m_is_opened = false;
}

RM_FileHandle::~RM_FileHandle() {
    m_is_opened = false;
}

RM_FileHandle& RM_FileHandle::operator= (const RM_FileHandle &fileHandle){
    if (this != &fileHandle){
        this->m_pf_filehandle = fileHandle.m_pf_filehandle;
        this->m_is_opened = fileHandle.m_is_opened;
        memcpy(&m_file_hdr,
               &fileHandle.m_file_hdr,
               sizeof(struct RM_FileHdr));
    }
    return (*this);
}

RC RM_FileHandle::GetRec(const RID &rid, RM_Record &rec) const {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;

    int pageNum = 0;
    int slotNum = 0;

    if (slotNum >= header.m_records_per_page)
        return RM_INVALIDRECORD;

    if ((rc = GetPageNumAndSlotNum(rid, pageNum, slotNum)))
        return rc;

    PF_PageHandle ph;
    if ((rc = m_pf_filehandle.GetThisPage(pageNum, ph)))
        return rc;

    char *bitmap = NULL;
    char *records = NULL;
    bool hasValidRecord = false;
    int recordSize = m_file_hdr.GetRecordSize();

    rc = GetRecordsAndBitmap(ph, bitmap, records);
    
    if (rc == 0){
        rc = CheckBitSet(bitmap, slotNum, hasValidRecord);
    }

    if (rc == 0) {
        if (!hasValidRecord) {
            rc = RM_INVALIDRECORD;
        } else {
            char *record_pos = records + recordSize * slotNum;
            rc = rec.SetRecord(rid, record_pos);
        }
    }

    RC rc2;
    if ((rc2 = m_pf_filehandle.UnpinPage(pageNum)))
        return rc2;
    return (rc);
}

RC InsertRec (char *pData, RID &rid) {
    if (pData == NULL)
        return RM_INVALIDRECORD;

    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;

    RC rc = OK_RC;
    if ((rc = rid.IsInvalidRID()))
        return rc;

    PF_PageHandle ph;
    PageNum pageNum = m_file_hdr.GetValidPage();
    SlotNum slotNum = -1;
    if (pageNum == NO_MORE_FREE_PAGE) {
        if ((rc = AllocateNewPage(ph)))
            return rc;
        if ((rc = ph.GetPageNum(pageNum)))
            return rc;
        m_file_hdr.SetValidPage(pageNum);
    } else {
        if ((rc == m_pf_filehandle.GetThisPage(pageNum, ph)))
            return rc;
        pageNum = m_file_hdr.GetValidPage();
    }

    char* bitmap = NULL;
    char* location = NULL;
    int recordSize = m_file_hdr.GetRecordSize();
    int bitmapSize = m_file_hdr.GetBitmapSize();
    rc = ph.GetData(bitmap);
   
    if (rc == OK_RC)
        GetFirstZeroBitSlot(bitmap, slotNum);

    if (rc == OK_RC)
        rc = SetBit(bitmap, slotNum);

    if (rc == OK_RC)
        rc = UpdateFileHeader();

    if (rc == OK_RC) {
        location = bitmap + (bitmapSize + recordSize * slotNum);
        memcpy(location, pData, recordSize);
        rid.SetValue(pageNum, slotNum);
    }

    RC rc2 = OK_RC;
    if ((rc = m_pf_filehandle.MarkDirty(pageNum)) || 
        (rc = m_pf_filehandle.UnpinPage(pageNum)))
        return rc2;

    return rc;
}

RC RM_FileHandle::DeleteRec (RID &rid) {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;
    
    PageNum pageNum = -1;
    SlotNum slotNum = -1;
    RC rc = OK_RC;
    if ((rc = GetPageNumAndSlotNum(rid, pageNum, slot)))
        return rc;

    if (slotNum >= header.m_records_per_page)
        return RM_INVALIDRECORD;

    PF_PageHandle ph;
    if ((rc = m_pf_filehandle.GetThisPage(pageNum, ph)))
        return rc;

    char* bitmap = NULL;
    bool hasValue = false;
    rc = ph.GetData(bitmap)

    if (rc == OK_RC)
        rc = CheckBitSet(bitmap, slotNum, hasValue);

    if (rc == OK_RC && !hasValue)
        rc = RM_INVALIDRECORD;

    if (rc == OK_RC)
        rc = ResetBit(bitmap, slotNum);

    if (rc == OK_RC && IsPageEmpty(bitmap)) {
        rc = m_pf_filehandle.DisposePage(pageNum);
        if (rc == OK_RC) {
            m_file_hdr.DecTotalPages();
            rc = UpdateFileHeader();
        }
    }

    RC rc2 = OK_RC;
    if ((rc2 = m_pf_filehandle.MarkDirty(pageNum)) ||
        (rc2 = m_pf_filehandle.UnpinPage(pageNum)))
        return rc2;
    return rc;
}

RC RM_FileHandle::UpdateRec (RM_Record &rec) {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;
    PageNum pageNum = -1;
    SlotNum slotNum = -1;
    char* pData = NULL;
    RID rid;
    
    if ((rc = rec.GetRid(rid)))
        return rc;

    if ((rc = rec.GetData(pData)))
        return rc;

    if ((rc = rid.GetPageNum(pageNum)))
        return rc;

    if ((rc = rid.GetSlotNum(slotNum)))
        return rc;

    PF_PageHandle ph;
    if ((rc = m_pf_filehandle.GetThisPage(pageNum, ph)))
        return rc;

    char* bitmap = NULL;
    char* localtion = NULL;
    bool hasValue = false;
    int bitmapSize = m_file_hdr.GetBitmapSize();
    int recordSize = m_file_hdr.GetRecordSize();
    rc = ph.GetData(bitmap);

    if (rc == OK_RC)
        rc = CheckBitSet(bitmap, slotNum, hasValue);

    if (rc == OK_RC && !hasValue)
        rc = RM_INVALIDRECORD;

    if (rc == OK_RC) {
        location = bitmap + (bitmapSize + 
                             recordSize * slotNum);
        memcpy(location, pData, recordSize);
    }

    RC rc2 = OK_RC;
    if ((rc2 = m_pf_filehandle.MarkDirty(pageNum)) ||
            (rc2 = m_pf_filehandle.UnpinPage(pageNum)))
        return rc2;

    return rc;
}

RC RM_FileHandle::ForcePages (PageNum pageNum = ALL_PAGES) {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;
    return m_pf_filehandle.ForcePages(pageNum);
}

RC RM_FileHandle::GetNextRecord(PageNum page,
                                SlotNum slot,
                                RM_Record &rec,
                                PF_PageHandle &ph,
                                bool nextPage) {
}

RC RM_FileHandle::FirstRecord()


RC RM_FileHandle::UpdateFileHeader(){
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;

    PF_PageHandle ph;
    RC rc = OK_RC;
    RC rc2 = OK_RC;
    if ((rc = m_pf_filehandle.GetFirstPage(ph))){
        return rc;
    }

    bool hasValue = false;
    PageNum pageNum = -1;
    char* bitmap = NULL;
    do {
        rc = ph.GetData(bitmap);
        if (rc == OK_RC)
            rc = ph.GetPageNum(pageNum);

        if (rc == OK_RC && !IsPageEmpty(bitmap)) {
            m_file_hdr.SetValidPage(pageNum);
            hasValue = true;
        }

        if ((rc2 = m_pf_filehandle.UnpinPage(pageNum))) {
            return rc2;
        }
    } while (!hasValue && !(rc = m_pf_filehandle.GetNextPage(ph)));

    if (!hasValue) {
        m_file_hdr.SetValidPage(NO_FREE_PAGES);
    }
    return rc;
}

bool RM_FileHandle::IsPageEmpty(char* bitmap) {
    int recordPerPage = m_file_hdr.GetRecordPerPage();
    for (int slot = 0; slot < recordPerPage; ++slot) {
        int bucket = slot/8;
        int offset = 8 - (slot - bucket * 8);
        bool isSet = ((bitmap[bucket] >> (offset)) & 0x01) == 1;
        if (isSet)
            return false;
    }
    return true;
}

RC RM_FileHandle::GetFirstZeroBitSlot(char* bitmap, SlotNum &slotNum) {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;

    int recordPerPage = m_file_hdr.GetRecordPerPage();
    for (int slot = 0; slot < recordPerPage; ++slot) {
        int bucket = slot/8;
        int offset = 8 - (slot - bucket * 8);
        bool isSet = ((bitmap[bucket] >> (offset)) & 0x01) == 1;
        if (!isSet) {
            slotNum = slot;
            return (0);
        }
    }
    return RM_PAGEFULL;
}

RC RM_FileHandle::SetBit(char *bitmap, int slotNum){
    int recordPerPage = m_file_hdr.GetRecordPerPage();
    if (slotNum >= recordPerPage)
        return (RM_INVALIDBITOPERATION);
    int bucket = slotNum / 8;
    int offset = slotNum - 8 * bucket;

    bitmap[bucket] = (bitmap[bucket] & (~(0x1 << offset)));
    if (slotNum == recordPerPage - 1) {
        m_file_hdr.SetValidPage(NO_MORE_FREE_PAGE);
    }
    return (0);
}

RC RM_FileHandle::ResetBit(char *bitmap, int slotNum){
    int recordPerPage = m_file_hdr.GetRecordPerPage();
    if (slotNum >= recordPerPage)
        return (RM_INVALIDBITOPERATION);
    int bucket = slotNum / 8;
    int offset = slotNum - 8 * bucket;

    bitmap[bucket] = (bitmap[bucket] | 0x1 << offset);
    if (slotNum == recordPerPage - 1) {
        m_file_hdr.SetValidPage(NO_MORE_FREE_PAGE);
    }
    return (0);
}

RC RM_FileHandle::AllocateNewPage(PF_PageHandle &ph) {
    if (!m_is_opened)
        return RM_INVALIDFILEHANDLE;

    RC rc = 0;
    PageNum newPageNum = -1;
    if ((rc = m_pf_filehandle.AllocatePage(ph)))
        return rc;
    m_file_hdr.IncTotalPages();
    return 0;
}

RC RM_FileHandle::Initialize(PF_FileHandle &fh) {
    PF_PageHandle ph;
    PageNum pageNum;
    if ((rc = fh.GetFirstPage(ph))) {
        return rc;
    }
    
    char *hdr = NULL;
    if ((rc = ph.GetPageNum(pageNum))) {
        return rc;
    }

    if ((rc = ph.GetData(hdr))) {
        return rc;
    }

    memcpy(&m_file_hdr, hdr, sizeof (struct RM_FileHdr));
    this->m_pf_filehandle = fh;
    this->m_is_opened = true;

    if ((rc = fh.UnpinPage(pageNum)))
        return rc;
    return (0);
}

RC RM_FileHandle::CheckBitSet(char* bitmap,
                              SlotNum slotNum,
                              bool &hasValidRecord) {
    int bucket = slotNum / 8;
    int offset = 8 - (slotNum - bucket * 8);
    hasValidRecord = ((bitmap[bucket] >> (offset)) & 0x1) == 1;
    return (0);
}

RC RM_FileHandle::GetPageNumAndSlotNum(const RID &rid,
                                       PageNum pageNum,
                                       SlotNum slotNum) {
    if ((rc = rid.IsInvalidRID()) ||
        (rc = rid.GetPageNum(pageNum)) ||
        (rc = rid.GetSlotNum(slotNum)))
        return rc;
    return (0); 
}

RC RM_FileHandle::GetRecordsAndBitmap(const PF_PageHandle& ph,
                                      char *&bitmap,
                                      char *&records) {
    char *data = NULL;
    if ((rc = ph.GetData(data)))
        return rc;
    bitmap = data;
    records = bitmap + m_file_hdr.GetBitmapSize();
    return (0);
}

int RM_FileHandle::CalRecordsPerPage(int recordSize) {
    return std::floor((PF_PAGE_SIZE * 1.0) / (recordSize * 1.0 + 1.0/8));
}

int RM_FileHandle::CalBitmapSizeInByte(int m_records_per_page) {
    int size = static_cast<int>(std::ceil((m_records_per_page * 1.0) / 8));
    if (size * 8 < m_records_per_page)
        ++size;
    return size;
}
