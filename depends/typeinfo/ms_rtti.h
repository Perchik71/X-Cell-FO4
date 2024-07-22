// Author: Nukem9
// https://github.com/Nukem9/SkyrimSETest/blob/master/skyrim64_test/src/typeinfo/ms_rtti.h

#pragma once

#include <vector>

namespace msrtti
{
	namespace detail { struct complete_object_locator; }

	struct info
	{
		uintptr_t vtable_address;					// Address in .rdata section
		uintptr_t vtable_offset;					// Offset of this vtable in complete class (from top)
		uint64_t vfunction_total;					// Number of contiguous functions
		const char* name;							// Demangled
		const char* raw_name;						// Mangled
		detail::complete_object_locator* locator;	//
	};

	struct section
	{
		uintptr_t base;
		uintptr_t end;
	};

	void init(uintptr_t base, const section& code, const section& data, const section& rdata);
	void dump(FILE* file);
	const info* find(const char* name, bool exact = true);
	vector<const info*> find_all(const char* name, bool exact = true);

	namespace detail
	{
		struct type_descriptor;
		struct pmd;
		struct complete_object_locator;
		struct class_hierarchy_descriptor;
		struct base_class_array;
		struct base_class_descriptor;

		template<typename T>
		struct rva
		{
			uint32_t offset;

			inline uintptr_t address()
			{
				return (uintptr_t)GetModuleHandleA(NULL) + offset;
			}

			inline T get()
			{
				return (T)address();
			}
		};

		// Also known as `class type_info`
		struct type_descriptor
		{
			void* vftable;		// const type_info::`vftable'
			uint64_t unknown;	// CRT internal
			char name[1];
		};

		struct pmd
		{
			int mdisp;	// Member displacement (vftable offset in the class itself)
			int pdisp;	// Vbtable displacement (vbtable offset, -1: vftable is at displacement PMD.mdisp inside the class)
			int vdisp;	// Displacement inside vbtable
		};

		struct complete_object_locator
		{
			enum : uint32_t
			{
				col_signature_32 = 0,
				col_signature_64,
			};

			uint32_t signature;								// 32-bit zero, 64-bit one
			uint32_t offset;								// Offset of this vtable in the complete class
			uint32_t cd_offset;								// Constructor displacement offset
			rva<type_descriptor*> type_desc;				// TypeDescriptor of the complete class
			rva<class_hierarchy_descriptor*> class_desc;	// Describes inheritance hierarchy
		};

		struct class_hierarchy_descriptor
		{
			enum : uint32_t
			{
				hcd_no_inheritance = 0,
				hcd_multiple_inheritance = 1,
				hcd_virtual_inheritance = 2,
				hcd_ambiguous_inheritance = 4,
			};

			uint32_t signature;							// Always zero or one
			uint32_t attributes;						// Flags
			uint32_t num_base_classes;					// Number of classes in BaseClassArray
			rva<base_class_array*> base_class_array;	// BaseClassArray
		};

#pragma warning(push)
#pragma warning(disable: 4200) // nonstandard extension used: zero-sized array in struct/union
		struct base_class_array
		{
			uint32_t array_of_base_class_desc[]; // BaseClassDescriptor *
		};
#pragma warning(pop)

		struct base_class_descriptor
		{
			enum
			{
				bcd_not_visible = 1,
				bcd_ambiguous = 2,
				bcd_private = 4,
				bcd_priv_or_prot_base = 8,
				bcd_virtual = 16,
				bcd_nonpolymorphic = 32,
				bcd_has_hierarchy_descriptor = 64,
			};

			rva<type_descriptor*> type_desc;	// Type descriptor of the class
			uint32_t num_contained_bases;		// Number of nested classes following in the Base Class Array
			pmd disp;							// Pointer-to-member displacement info
			uint32_t attributes;				// Flags (BaseClassDescriptorFlags)
		};

		bool is_within_rdata(uintptr_t address);
		bool is_within_code(uintptr_t address);
		bool is_valid_col(complete_object_locator* locator);
		const char* strcasestr(const char* string, const char* substring);
	}
}