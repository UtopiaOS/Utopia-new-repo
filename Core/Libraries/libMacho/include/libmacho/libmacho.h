#include "libC/internal/attributes.h"

#define __NEED_Int32
#define __NEED_UInt32
#define __NEED_UInt64
#define __NEED_UInt16
#define __NEED_UIntMax
#define __NEED_UInt8
#define __NEED_Int64
#include <covenant/bits/everytype.h>
#include <covenant/bool.h>

// Inspired by: https://github.com/ravynsoft/ravynos/blob/main/sys/sys/elf64.h
typedef UInt64 Macho64Addr;
typedef UInt16 Macho64Half;
typedef UInt64 Macho64Off;
typedef Int32  Macho64Sword;
typedef Int64  Macho64Sxword;
typedef UInt32 Macho64Word;
typedef UInt64 Macho64Lword;
typedef UInt64 Macho64Xword;

/* Decoded from https://github.com/ravynsoft/ravynos/blob/main/sys/sys/elf_generic.h */
/* Those are useless for us, but the idea is the same for our declarations... */
// Additional references: https://github.com/ravynsoft/ravynos/blob/4379804dbb4453d866987a7fa34f48f25c258719/sys/sys/elf64.h
typedef Macho64Word Macho64Symbol;

UTOPIA_ENUM(Int32, MachoCPUType){
    macho_cpu_type_x86_64 = 0x01000007,
};

UTOPIA_ENUM(Int32, MachoCPUSubType){
    macho_cpu_type_x86_64_all = 3,
};

UTOPIA_ENUM(UInt32, MachoFileType){
    macho_file_type_object = 1,
    macho_file_type_executable = 2,
    macho_file_type_dynamic_library = 6,
    macho_file_type_dynamic_linker = 7,
};

UTOPIA_OPTIONS(UInt32, MachoHeaderFlags)
{
    macho_header_flag_no_undefined_symbols = 1ULL << 0,
    macho_header_flag_dynamically_linked = 1ULL << 2,
    macho_header_flag_use_two_level_namespace = 1ULL << 7,
    macho_header_flag_pie = 1ULL << 21,
};

UTOPIA_PACKED_STRUCT(MachoHeader) {
    UInt32 magic;
    MachoCPUType cpu_type;
    MachoCPUSubType cpu_subtype;
    MachoFileType file_type;
    UInt32 command_count;
    UInt32 total_command_size;
    MachoHeaderFlags flags;
    UInt32 reserved;
    char load_commands[];
};

#define MACHO_MAGIC_64 (0xfeedfacfU)

UTOPIA_ENUM(UInt32, MachoLoadCommandType) {
    macho_load_command_type_symbol_table_info = 0x02,
    macho_load_command_type_unix_thread = 0x05,
    macho_load_command_type_dynamic_symbol_table_info = 0x0b,
    macho_load_command_type_load_dylib = 0x0c,
    macho_load_command_type_load_dynamic_linker = 0x0e,
    macho_load_command_type_segment_64 = 0x19,
    macho_load_command_type_compressed_dynamic_linker_info_only = 0x80000022,
    macho_load_command_type_entry_point = 0x80000028, // ?
};

UTOPIA_PACKED_STRUCT(MachoLoadCommand) {
    MachoLoadCommandType type;
    UInt32 size;
};


UTOPIA_PACKED_STRUCT(MachoLoadCommandSegment64) {
    MachoLoadCommand base;
    char segment_name[16];
    UInt64 memory_address;
    UInt64 memory_size;
    UInt64 file_offset;
    UInt64 file_size;
    Int32 maximum_memory_protection;
    Int32 initial_memory_protection;
    UInt32 section_count;
    UInt32 flags;
};

UTOPIA_PACKED_STRUCT(MachoSection64) {
    char section_name[16];
    char segment_name[16];
    UInt64 memory_address;
    UInt64 size;
    UInt32 file_offset;
    UInt32 alignment;
    UInt32 relocations_file_offset;
    UInt32 relocation_count;
    UInt32 flags;
    UInt32 reserved1;
    UInt32 reserved2;
    UInt32 reserved3;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandThread) {
    MachoLoadCommand base;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandDynamicLinker) {
    MachoLoadCommand base;
    UInt32 name_offset;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandDylib) {
    MachoLoadCommand base;
    UInt32 name_offset;
    UInt32 timestamp;
    UInt32 current_version;
    UInt32 compat_version;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandSymbolTableInfo) {
    MachoLoadCommand base;

    UInt32 symbol_table_offset;
    UInt32 symbol_table_entry_count;

    UInt32 string_table_offset;
    UInt32 string_table_size;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandDynamicSymbolTableInfo) {
	MachoLoadCommand base;

	UInt32 local_symbols_start_index;
	UInt32 local_symbol_count;

	UInt32 external_symbols_start_index;
	UInt32 external_symbol_count;

	UInt32 undefined_symbols_start_index;
	UInt32 undefined_symbol_count;

	UInt32 table_of_contents_offset;
	UInt32 table_of_contents_entry_count;

	UInt32 module_table_offset;
	UInt32 module_table_entry_count;

	UInt32 external_reference_table_offset;
	UInt32 external_reference_table_entry_count;

	UInt32 indirect_symbol_table_offset;
	UInt32 indirect_symbol_table_entry_count;

	UInt32 external_relocations_offset;
	UInt32 external_relocation_count;

	UInt32 local_relocations_offset;
	UInt32 local_relocation_count;
};

UTOPIA_ENUM(UInt8, MachoSymbolEntryTableType) {
	macho_symbol_table_entry_type_undefined = 0,
	macho_symbol_table_entry_type_absolute  = 1,
	macho_symbol_table_entry_type_indirect  = 5,
	macho_symbol_table_entry_type_prebound  = 6,
	macho_symbol_table_entry_type_section   = 7,
};

UTOPIA_ENUM(UInt8, MachoSymbolTableEntrySection) {
	macho_symbol_table_entry_section_none = 0,
	// all other values are valid section indicies
};

UTOPIA_ENUM(UInt8, MachoSymbolTableEntryLibraryIndex) {
	macho_symbol_table_entry_library_index_self           = 0,
	macho_symbol_table_entry_library_index_dynamic_lookup = 0xfe,
	macho_symbol_table_entry_library_index_executable     = 0xff,
};

UTOPIA_PACKED_STRUCT(MachoSymbolTableEntry) {
	UInt32 string_table_name_offset;
	UInt8 type;
	UInt8 section;
	UInt16 description;
	Int64 value;
};

UTOPIA_ALWAYS_INLINE MachoSymbolEntryTableType macho_symbol_table_entry_get_type(UInt8 type_field) {
	return (type_field >> 1) & 7;
};

UTOPIA_ALWAYS_INLINE bool macho_symbol_table_entry_is_external(UInt8 type_field) {
	return type_field & 1;
};

UTOPIA_ALWAYS_INLINE bool macho_symbol_table_entry_is_private_extern(UInt8 type_field) {
	return (type_field >> 4) & 1;
};

UTOPIA_ALWAYS_INLINE UInt8 macho_symbol_table_entry_library_index(UInt16 description_field) {
	return description_field >> 8;
};

UTOPIA_ALWAYS_INLINE bool macho_symbol_table_entry_library_index_is_special(UInt8 library_index) {
	switch (library_index) {
		case macho_symbol_table_entry_library_index_self:
		case macho_symbol_table_entry_library_index_dynamic_lookup:
		case macho_symbol_table_entry_library_index_executable:
			return true;
		default:
			return false;
	}
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandEntryPoint) {
	MachoLoadCommand base;

	Int64 entry_offset;
	Int64 stack_size;
};

UTOPIA_PACKED_STRUCT(MachoLoadCommandCompressedDynamicLinkerInfo) {
	MachoLoadCommand base;

	UInt32 rebase_info_offset;
	UInt32 rebase_info_size;

	UInt32 bind_info_offset;
	UInt32 bind_info_size;

	UInt32 weak_bind_info_offset;
	UInt32 weak_bind_info_size;

	UInt32 lazy_bind_info_offset;
	UInt32 lazy_bind_info_size;

	UInt32 export_info_offset;
	UInt32 export_info_size;
};

UTOPIA_ENUM(UInt8, MachoRelocationType) {
	macho_relocation_type_pointer = 1,
	macho_relocation_type_text_absolute_32 = 2,
	macho_relocation_type_text_pc_relative_32 = 3,
};

UTOPIA_ENUM(UInt8, MachoRebaseOPCode) {
	macho_rebase_opcode_done = 0,
	macho_rebase_opcode_set_type_immediate = 1,
	macho_rebase_opcode_set_segment_immediate_and_offset_uleb = 2,
	macho_rebase_opcode_add_address_uleb = 3,
	macho_rebase_opcode_add_immediate_scaled = 4,
	macho_rebase_opcode_perform_rebase_immediate_times = 5,
	macho_rebase_opcode_perform_rebase_uleb_times = 6,
	macho_rebase_opcode_perform_rebase_add_uleb = 7,
	macho_rebase_opcode_perform_rebase_uleb_times_skipping_uleb = 8,
};

UTOPIA_ENUM(UInt8, MachoBindCode) {
	macho_bind_opcode_done = 0,
	macho_bind_opcode_set_dylib_ordinal_immediate = 1,
	macho_bind_opcode_set_dylib_ordinal_uleb = 2,
	macho_bind_opcode_set_dylib_special_immediate = 3,
	macho_bind_opcode_set_symbol_trailing_flags = 4,
	macho_bind_opcode_set_type_immediate = 5,
	macho_bind_opcode_set_addend_sleb = 6,
	macho_bind_opcode_set_segment_immediate_and_offset_uleb = 7,
	macho_bind_opcode_add_address_uleb = 8,
	macho_bind_opcode_perform_bind = 9,
	macho_bind_opcode_perform_bind_add_address_uleb = 10,
	macho_bind_opcode_perform_bind_add_address_immediate_scaled = 11,
	macho_bind_opcode_perform_bind_uleb_times_skipping_uleb = 12,
	macho_bind_opcode_threaded = 13,
};

UTOPIA_ENUM(UInt8, MachoBindSubOPCodeThreaded) {
	macho_bind_subopcode_threaded_set_bind_ordinal_table_size_uleb = 0,
	macho_bind_subopcode_threaded_apply = 1,
};

UTOPIA_ENUM(UInt8, MachoExportSymbolKind) {
	macho_export_symbol_kind_regular = 0,
	macho_export_symbol_kind_thread_local = 1,
	macho_export_symbol_kind_absolute = 2,
};

UTOPIA_ENUM(UInt8, MachoExportSymbolFlags) {
	macho_export_symbol_flag_weak = 1 << 0,
	macho_export_symbol_flag_reexport = 1 << 1,
	macho_export_symbol_flag_stub_and_resolver = 1 << 2,
};

UTOPIA_ALWAYS_INLINE UInt8 macho_relocation_instruction_get_opcode(UInt8 byte) {
	return byte >> 4;
};

UTOPIA_ALWAYS_INLINE UInt8 macho_relocation_instruction_get_immediate(UInt8 byte) {
	return byte & 0x0f;
};

UTOPIA_ALWAYS_INLINE MachoExportSymbolKind macho_export_flags_get_kind(UIntMax raw_flags) {
	return raw_flags & 3ULL;
};

UTOPIA_ALWAYS_INLINE MachoExportSymbolFlags macho_export_flags_get(UIntMax raw_flags) {
	return raw_flags >> 2;
};
