#include "rm_rid.h"

RID::RID () {
    pageNum = INVALID_PAGE;
    slotNum = INVALID_SLOT;
}

RID::~RID () {
}

RID::RID (PageNum pageNum, SlotNum slotNum) {
    this->pageNum = pageNum;
    this->slotNum = slotNum;
}

inline bool RID::operator== (const RID &rid) const {
    return rid.pageNume == pageNum && rid.slotNum == slotNum;
}

inline RC RID::GetPageNum (PageNum &pageNum) const {
    pageNum = this->pageNum;
    return (0);
}

inline RC RID::GetSlotNum (SlotNum &slotNum) const {
    slotNum = this->slotNum;
    return (0);
}

inline void RID::SetValue (PageNum page, SlotNum slot){
    this->pageNum = page;
    this->slotNum = slot;
}

inline RC RID::IsInvalidRID () const {
    if (pageNum == INVALID_PAGE || slotNum == INVALID_SLOT)
        return RM_INVALIDRID;
    return OK_RC;
}
