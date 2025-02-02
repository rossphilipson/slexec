/*
 * Copyright (c) 2022, Oracle and/or its affiliates.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _SLR_TABLE_H
#define _SLR_TABLE_H

/* Put this in efi.h if it becomes a standard */
#define SLR_TABLE_GUID				EFI_GUID(0x877a9b2a, 0x0385, 0x45d1, 0xa0, 0x34, 0x9d, 0xac, 0x9c, 0x9e, 0x56, 0x5f)

/* SLR table header values */
#define SLR_TABLE_MAGIC		0x4452544d
#define SLR_TABLE_REVISION	1

/* SLR defined architectures */
#define SLR_INTEL_TXT		1
#define SLR_AMD_SKINIT		2

/* Log formats */
#define SLR_DRTM_TPM12_LOG	1
#define SLR_DRTM_TPM20_LOG	2

/* DRTM Policy Entry Flags */
#define SLR_POLICY_FLAG_MEASURED	0x1
#define SLR_POLICY_IMPLICIT_SIZE	0x2

/* Array Lengths */
#define TPM_EVENT_INFO_LENGTH		32
#define TXT_VARIABLE_MTRRS_LENGTH	32

/* Tags */
#define SLR_ENTRY_INVALID	0x0000
#define SLR_ENTRY_DL_INFO	0x0001
#define SLR_ENTRY_LOG_INFO	0x0002
#define SLR_ENTRY_ENTRY_POLICY	0x0003
#define SLR_ENTRY_INTEL_INFO	0x0004
#define SLR_ENTRY_AMD_INFO	0x0005
#define SLR_ENTRY_ARM_INFO	0x0006
#define SLR_ENTRY_EFI_INFO	0x0007
#define SLR_ENTRY_EFI_CONFIG	0x0008
#define SLR_ENTRY_END		0xffff

/* Entity Types */
#define SLR_ET_UNSPECIFIED	0x0000
#define SLR_ET_SLRT		0x0001
#define SLR_ET_BOOT_PARAMS	0x0002
#define SLR_ET_SETUP_DATA	0x0003
#define SLR_ET_CMDLINE		0x0004
#define SLR_ET_EFI_MEMMAP	0x0005
#define SLR_ET_INITRD		0x0006
#define SLR_ET_TXT_OS2MLE	0x0010
#define SLR_ET_UNUSED		0xffff

#ifndef __ASSEMBLY__

/*
 * Primary SLR Table Header
 */
struct slr_table {
	u32 magic;
	u16 revision;
	u16 architecture;
	u32 size;
	u32 max_size;
	/* entries[] */
} __packed;

/*
 * Common SLRT Table Header
 */
struct slr_entry_hdr {
	u16 tag;
	u16 size;
} __packed;

/*
 * DRTM Dynamic Launch Configuration
 */
struct slr_entry_dl_info {
	struct slr_entry_hdr hdr;
	u64 dl_handler;
	u64 dce_base;
	u32 dce_size;
	u64 dlme_entry;
} __packed;

/*
 * TPM Log Information
 */
struct slr_entry_log_info {
	struct slr_entry_hdr hdr;
	u16 format;
	u16 reserved;
	u64 addr;
	u32 size;
} __packed;

/*
 * DRTM Measurement Policy
 */
struct slr_entry_policy {
	struct slr_entry_hdr hdr;
	u16 revision;
	u16 nr_entries;
	/* policy_entries[] */
} __packed;

/*
 * DRTM Measurement Entry
 */
struct slr_policy_entry {
	u16 pcr;
	u16 entity_type;
	u16 flags;
	u16 reserved;
	u64 entity;
	u64 size;
	char evt_info[TPM_EVENT_INFO_LENGTH];
} __packed;

/*
 * Secure Launch defined MTRR saving structures
 */
struct txt_mtrr_pair {
	u64 mtrr_physbase;
	u64 mtrr_physmask;
} __packed;

struct txt_mtrr_state {
	u64 default_mem_type;
	u64 mtrr_vcnt;
	struct txt_mtrr_pair mtrr_pair[TXT_VARIABLE_MTRRS_LENGTH];
} __packed;

/*
 * Intel TXT Info table
 */
struct slr_entry_intel_info {
	struct slr_entry_hdr hdr;
	u64 saved_misc_enable_msr;
	struct txt_mtrr_state saved_bsp_mtrrs;
} __packed;

/*
 * AMD SKINIT Info table
 */
struct slr_entry_amd_info {
	struct slr_entry_hdr hdr;
} __packed;

/*
 * ARM DRTM Info table
 */
struct slr_entry_arm_info {
	struct slr_entry_hdr hdr;
} __packed;

struct slr_entry_efi_config {
	struct slr_entry_hdr hdr;
	u32 identifier;
	u16 reserved;
	u16 nr_entries;
	/* efi_cfg_entries[] */
} __packed;

struct efi_cfg_entry {
	u64 cfg; /* address or value */
	u32 size;
	char evt_info[TPM_EVENT_INFO_LENGTH];
} __packed;

static inline void *slr_end_of_entrys(struct slr_table *table)
{
	return (((void *)table) + table->size);
}

static inline struct slr_entry_hdr *
slr_next_entry(struct slr_table *table,
		struct slr_entry_hdr *curr)
{
	struct slr_entry_hdr *next = (struct slr_entry_hdr *)
				((u8 *)curr + curr->size);

	if ((void *)next >= slr_end_of_entrys(table))
		return NULL;
	if (next->tag == SLR_ENTRY_END)
		return NULL;

	return next;
}

static inline struct slr_entry_hdr *
slr_next_entry_by_tag(struct slr_table *table,
		      struct slr_entry_hdr *entry,
		      u16 tag)
{
	if (!entry) /* Start from the beginning */
		entry = (struct slr_entry_hdr *)(((u8 *)table) + sizeof(*table));

	for ( ; ; ) {
		if (entry->tag == tag)
			return entry;

		entry = slr_next_entry(table, entry);
		if (!entry)
			return NULL;
	}

	return NULL;
}

static inline int
slr_add_entry(struct slr_table *table,
	      struct slr_entry_hdr *entry)
{
	if ((table->size + entry->size) > table->max_size)
		return -1;

	sl_memcpy((u8 *)table + table->size, entry, entry->size);
	table->size += entry->size;

	return 0;
}

static inline void
slr_init_table(struct slr_table *slrt, u16 architecture, u32 max_size)
{
	slrt->magic = SLR_TABLE_MAGIC;
	slrt->revision = SLR_TABLE_REVISION;
	slrt->architecture = architecture;
	slrt->size = sizeof(*slrt);
	slrt->max_size = max_size;
}

#endif /* !__ASSEMBLY */

#endif /* _SLR_TABLE_H */
