#include "rm.h"

RM_FileScan::RM_FileScan() {
    attribute_type_ = INT;
    attribute_length_ = 0;
    attribute_offset_ = 0;
    compare_op_ = NO_OP;
    compare_val_ = NULL;
    hint_ = NO_HINT;
    opened_ = false;
    scan_end_ = true;
}


RM_FileScan::~RM_FileScan() {
    attribute_type_ = INT;
    attribute_length_ = 0;
    attribute_offset_ = 0;
    compare_op_ = NO_OP;
    compare_val_ = NULL;
    hint_ = NO_HINT;
    opened_ = false;
    scan_end_ = true;
}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle,
                         AttrType      attrType,
                         int           attrLength,
                         int           attrOffset,
                         CompOp        compOp,
                         void          *value,
                         ClientHint    pinHint = NO_HINT) {
    if(opened_)
        return (RM_INVALIDSCAN);

    if(fileHandle.isValidFileHeader())
        file_handle_ = fileHandle;
    else
        return (RM_INVALIDFILEHANDLE);
  
    if((rc = SetCompareOp(attrType, compOp)))
        return rc;

    int recordsize = fileHandle.GetRecordSize();
    if (attrOffset < 0 || attrLength < 0 ||
            attrOffset + attrLength > recordSize)
        return (RM_INVALIDSCAN);

    attribute_length_ = attrLength;
    attribute_offset_ = attrOffset;
    compare_val_ = operator new(attrLength);
    memcpy(compare_val_, value, attrLength);
    hint_ = pinHint;
    opened_ = true;
    scan_end_ = false;

    return OK_RC;
}

RC RM_FileScan::GetNextRec(RM_Record &rec){
    
}


RC RM_FileScan::SetCompareOp(AttrType attrType, CompOp compOp) {
    switch(compOp) {
        case EQ_OP:
            switch(attrType) {
                case INT: compare_op_ = &equal_int; break;
                case FLOAT: compare_op_ = &equal_float; break;
                case STRING: compare_op_ = &equal_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case NE_OP:
            switch(attrType) {
                case INT: compare_op_ = &no_equal_int; break;
                case FLOAT: compare_op_ = &no_equal_float; break;
                case STRING: compare_op_ = &no_equal_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case LT_OP:
            switch(attrType) {
                case INT: compare_op_ = &less_than_int; break;
                case FLOAT: compare_op_ = &less_than_float; break;
                case STRING: compare_op_ = &less_than_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case GT_OP:
            switch(attrType) {
                case INT: compare_op_ = &greater_than_int; break;
                case FLOAT: compare_op_ = &greater_than_float; break;
                case STRING: compare_op_ = &greater_than_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case LE_OP:
            switch(attrType) {
                case INT: compare_op_ = &less_or_equal_int; break;
                case FLOAT: compare_op_ = &less_or_equal_float; break;
                case STRING: compare_op_ = &less_or_equal_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case GE_OP:
            switch(attrType) {
                case INT: compare_op_ = &greater_or_equal_int; break;
                case FLOAT: compare_op_ = &greater_or_equal_float; break;
                case STRING: compare_op_ = &greater_or_equal_str; break;
                default: return (RM_INVALIDSCAN);
            }
            break;
        case NO_OP:
            compare_op_ = NULL;
            break;
        default:
            return (RM_INVALIDSCAN);
    }
    return OK_RC;
}
