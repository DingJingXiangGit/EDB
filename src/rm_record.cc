#include "rm.h"

RM_Record::RM_Record() {
    rid = -1;
    data = NULL;
    size = INVALID_RECORD_SIZE;
}

RM_Record::~RM_Record() {
    delete data;
    data = NULL;
    size = INVALID_RECORD_SIZE;
    rid = -1;
}

RM_Record& RM_Record::RM_Record(RM_Record &record) {
    if (&record != this) {
        if (this->data != NULL)
            delete[] this->data;
        this->size = record.size;
        this->data = new char[this->size];
        memcpy(this->data, record.data, this.size);
        this->rid = record.rid;
    }
    return *this;
}

inline RC RM_Record::GetData(char *&pData) const {
    if (data == NULL || size == INVALID_RECORD_SIZE)
        return (RM_INVALIDRECORD);
    pData = this->data;
    return (0);
}

inline RC RM_Record::GetRid (RID &rid) const {
    RC rc;
    if ((rc = rid.IsInvalidRID()))
        return rc;
    rid = this->rid;
    return (0);
}

RC RM_Record::SetRecord (RID rec_rid, char *recData, int size) {
    RC rc;
    
    if ((rc = rec_rid.IsInvalidRID()))
        return rc;

    if (size == INVALID_RECORD_SIZE)
        return RM_BADRECORDSIZE;

    if (recData == NULL)
        return RM_INVALIDRECORD;

    if (this->data != NULL)
        delete[] this->data;

    this->rid = rec_rid;
    this->size = size;
    this->data = new char[size];
    memcpy(this->data, recData, size);
    return (0);
}
