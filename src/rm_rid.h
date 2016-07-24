#ifndef RID_H
#define RID_H

#include "redbase.h"
#include "rm_internal.h"

class RID {
    friend class RID;
    public:
        RID  ();  // Default constructor

        ~RID ();  // Destructor
        
        RID  (PageNum pageNum, SlotNum slotNum);
        
        bool operator== (const RID &rid) const;
        
        // Construct RID from page and slot number
        RC GetPageNum (PageNum &pageNum) const;  // Return page number
        
        RC GetSlotNum (SlotNum &slotNum) const;  // Return slot number

        RC SetValue (PageNum &pageNum, SlotNum &slotNum) ;
        
        RC IsInValidRID () const;

    private:
        PageNum pageNum;
        SlotNum slotNum;
};
#endif
