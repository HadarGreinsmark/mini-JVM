#include "class.h"

void print_class(FILE *stream, const Class *cls) {
	fprintf(stream, "File: %s\n", cls->file_name);
	fprintf(stream, "Minor number: %u \n", cls->minor_version);
	fprintf(stream, "Major number: %u \n", cls->major_version);
	fprintf(stream, "Constant pool size: %u \n", cls->const_pool_count);
	fprintf(stream, "Constant table size: %ub \n", cls->pool_size_bytes);
	fprintf(stream, "Printing constant pool of %d items...\n", cls->const_pool_count-1);

	Item *s;
	uint16_t i = 1; // constant pool indexes start at 1, get_item converts to pointer index
	while (i < cls->const_pool_count) {
		s = get_item(cls, i);
		fprintf(stream, "Item #%u %s: ", i, tag2str(s->tag));
		if (s->tag == STRING_UTF8) {
			fprintf(stream, "%s\n", s->value.string.value);
		} else if (s->tag == INTEGER) {
			fprintf(stream, "%d\n", s->value.integer);
		} else if (s->tag == FLOAT) {
			fprintf(stream, "%f\n", s->value.flt);
		} else if (s->tag == LONG) {
			fprintf(stream, "%ld\n", to_long(s->value.lng));
		} else if (s->tag == DOUBLE) {
			fprintf(stream, "%lf\n", to_double(s->value.dbl));
		} else if (s->tag == CLASS || s->tag == STRING) {
			fprintf(stream, "%u\n", s->value.ref.class_idx);
		} else if(s->tag == FIELD || s->tag == METHOD || s->tag == INTERFACE_METHOD || s->tag == NAME) {
			fprintf(stream, "%u.%u\n", s->value.ref.class_idx, s->value.ref.name_idx);
		} 
		i++;
	}

	fprintf(stream, "Access flags: %x\n", cls->flags);

	Item *cl_str = get_class_string(cls, cls->this_class);
	fprintf(stream, "This class: %s\n", cl_str->value.string.value);

	cl_str = get_class_string(cls, cls->super_class);
	fprintf(stream, "Super class: %s\n", cl_str->value.string.value);

	fprintf(stream, "Interfaces count: %u\n", cls->interfaces_count);

	fprintf(stream, "Printing %u interfaces...\n", cls->interfaces_count);
	if (cls->interfaces_count > 0) {
		Ref *iface = cls->interfaces;
		Item *the_class;
		uint16_t idx = 0;
		while (idx < cls->interfaces_count) {
			the_class = get_item(cls, iface->class_idx); // the interface class reference
			Item *item = get_item(cls, the_class->value.ref.class_idx);
			String string = item->value.string;
			fprintf(stream, "Interface: %s\n", string.value);
			idx++;
			iface = cls->interfaces + idx; // next Ref
		}
	}

	fprintf(stream, "Printing %d fields...\n", cls->fields_count);

	if (cls->fields_count > 0) {
		Field *field = cls->fields;
		uint16_t idx = 0;
		while (idx < cls->fields_count) {
			Item *name = get_item(cls, field->name_idx);
			Item *desc = get_item(cls, field->desc_idx);
			printf("%s %s\n", field2str(desc->value.string.value[0]), name->value.string.value);
			Attribute at;
			if (field->attrs_count > 0) {
				int aidx = 0;
				while (aidx < field->attrs_count) {
					at = field->attrs[aidx];
					Item *name = get_item(cls, at.name_idx);
					fprintf(stream, "\tAttribute name: %s\n", name->value.string.value);
					fprintf(stream, "\tAttribute length %d\n", at.length);
					fprintf(stream, "\tAttribute: %s\n", at.info);
					aidx++;
				}
			}
			idx++;
			field = cls->fields + idx;
		}
	}

	fprintf(stream, "Printing %u methods...\n", cls->methods_count);
	i = 0;
	if (cls->methods_count > 0) {
		Method *method = cls->methods;
		uint16_t idx = 0;
		while (idx < cls->methods_count) {
			Item *name = get_item(cls, method->name_idx);
			Item *desc = get_item(cls, method->desc_idx);
			printf("%s %s\n", name->value.string.value, desc->value.string.value);
			Attribute at;
			if (method->attrs_count > 0) {
				int aidx = 0;
				while (aidx < method->attrs_count) {
					at = method->attrs[aidx];
					Item *name = get_item(cls, at.name_idx);
					fprintf(stream, "\tAttribute name: %s", name->value.string.value);
					fprintf(stream, "\tAttribute length %d\n", at.length);
					fprintf(stream, "\tAttribute: %s\n", at.info);
					aidx++;
				}
			}
			idx++;
			method = cls->methods + idx;
		}
	}

	fprintf(stream, "Printing %u attributes...\n", cls->attributes_count);
	if (cls->attributes_count > 0) {
		Attribute at;
		int aidx = 0;
		while (aidx < cls->attributes_count) {
			at = cls->attributes[aidx];
			Item *name = get_item(cls, at.name_idx);
			fprintf(stream, "\tAttribute name: %s", name->value.string.value);
			fprintf(stream, "\tAttribute length %d\n", at.length);
			fprintf(stream, "\tAttribute: %s\n", at.info);
			aidx++;
		}
	}
}

