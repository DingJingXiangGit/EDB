#include "rm.h"
#include "rm_internal.h"

RM_Manager::RM_Manager(PF_Manager &pfm) : pf_manager(pfm){
}

RM_Manager::~RM_Manager(){
}

RC RM_Manager::CreateFile (const char *fileName, int recordSize) { 
    RC rc = 0;  // Initially set return code to zero.

    if (fileName == NULL)
        return RM_BADFILENAME;

    if (recordSize <= 0 || recordSize > PF_PAGE_SIZE)
        return RM_BADRECORDSIZE;
    
    int recordsPerPage = RM_FileHandle::CalRecordsPerPage(recordSize);
    int bitmapSizeInByte = RM_FileHandle::CalBitmapSizeInByte(recordsPerPage);
    int hdr_size = sizeof (struct RM_FileHdr);

    if ((PF_PAGE_SIZE - bitmapSizeInByte - hdr_size) / recordSize <= 0)
        return RM_BADRECORDSIZE;

    if ((rc = pf_manager.CreateFile(fileName))) {
        return (rc);
    }

    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    if ((rc = pf_manager.OpenFile(fileName, fileHandle))) {
        return rc;
    }

    if ((rc = fileHandle.AllocatePage(pageHandle))) {
        return rc;
    }

    int pageNum = 0;
    if ((rc = pageHandle.GetPageNum(pageNum))) {
        goto clean_and_exit;
    }

    struct RM_FileHdr *header = NULL;
    if ((rc = pageHandle.GetData(static_cast<char*>(header))))
        return rc;

    header->recordSize     = recordSize;
    header->recordsPerPage = (PF_PAGE_SIZE - bitmapSizeInByte - hdr_size) /
                             recordSize;
    header->bitmapSize     = bitmapSizeInByte;
    header->lastFreePage   = NO_MORE_FREE_PAGE;
    header->totalPages     = 1;

clean_and_exit:
    RC rc2;
    if ((rc2 = fileHandle.MarkDirty(pageNum)) ||
        (rc2 = fileHandle.UnpinPage(pageNum)) ||
        (rc2 = pf_manager.CloseFile(fileHandle))) {
        return rc2;
    }

    return rc;
}

RC RM_Manager::DestroyFile(const char* fileName) {
    RC rc;
    if (fileName == NULL)
        return RM_BADFILENAME;
    if ((rc = pf_manager.DestroyFile(fileName)))
        return rc;
    return (0);
}


RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    RC rc;

    if (fileName == NULL)
        return RM_BADFILENAME;

    if (fileHandle.IsOpened())
        return RM_INVALIDFILEHANDLE;

    PF_FileHandle pf_fh;
    if ((rc = pf_manager.OpenFile(fileName, pf_fh))) {
        return rc;
    }

    fileHandle.Initialize(pf_fh);;
    return (0);
}


RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    RC rc = 0;

    if((rc = pf_manager.CloseFile(fileHandle.pf_fileHandle))) {
        return rc;
    }

    return (0);
}
