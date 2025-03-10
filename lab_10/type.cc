// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "type.h"

std::shared_ptr<CType> const CType::kIntType = std::make_shared<CPrimaryType>(TypeKind::kInt, 4, 4);


CRecordType::CRecordType(llvm::StringRef name, const std::vector<Member> members, TagKind tag_kind)
    : CType(CType::TypeKind::kRecord, 0, 0), name_(name), members_(members), tag_kind_(tag_kind),
      max_size_member_rank_(-1)
{
    // TO DO: Compute size and algin.
    switch (tag_kind_) {
        case TagKind::kStruct:
            ComputeStructMemberOffsets();
            break;
        case TagKind::kUnion:
            ComputeUnionMemberOffsets();
            break;
    }
}

static int RoundUp(size_t base_addr, size_t align) {
    return (base_addr + align - 1) & ~(align - 1);
}

void CRecordType::ComputeStructMemberOffsets() {
    size_t offset = 0;
    int rank = 0;
    size_t total_size = 0;
    size_t max_element_align = 0;
    size_t max_element_size = 0;

    for (auto& member : members_) {
        size_t member_size = member.type->GetSize();
        size_t member_algin = member.type->GetAlign();
        // Compute the offset of current member.
        offset = RoundUp(offset, member_algin);
        member.offset = offset;
        member.rank = rank++;
        // Record the largest member's algin, 
        // as algin of the entire structure.
        max_element_align = std::max(max_element_align, member_algin);
        // Update offset, preparing for next member.
        offset += member_size;

        // Save the rank of max size member.
        if (max_element_size < member_size) {
            max_element_size = member_size;
            max_size_member_rank_ = member.rank;
        }
    }

    size_ = RoundUp(offset, max_element_align);
    align_ = max_element_align;
}

void CRecordType::ComputeUnionMemberOffsets() {
    int rank = 0;

    size_t max_element_align = 0;
    size_t max_element_size = 0;

    for (auto& member : members_) {
        size_t member_size = member.type->GetSize();
        size_t member_algin = member.type->GetAlign();

        member.offset = 0;
        member.rank = rank++;

        max_element_align = std::max(max_element_align, member_algin);

        // Save the rank of max size member.
        if (max_element_size < member_size) {
            max_element_size = member_size;
            max_size_member_rank_ = member.rank;
        }
    }

    size_ = RoundUp(max_element_size, max_element_align);
    align_ = max_element_align;
}
