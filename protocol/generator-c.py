#!/usr/bin/env python3

# Copyright 2022 Matthias Ringwald
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
# disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
# following disclaimer in the documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import os
import sys
import parser

## Collected data
protocol_version = 0
header = {}
components = {}
general_enums = {}
field_enums = {}


## C-generator
c_types = { 'bool' : 'bool',  'u8' : 'uint8_t' , 'u16' : 'uint16_t', 'u32' : 'uint32_t', 'string' : 'const char *', 'u8[]' : 'const uint8_t *',
            'buffer' : 'uint8_t *' , 'transport' : 'mb_transport_t *'}
c_size = { 'bool' : 1, 'u8' : 1, 'u16' : 2, 'u32' : 4, 'string' : 0, 'u8[]' : 0 , 'enum' : 1}

c_buffer_accessor = { 'bool' : '{buffer}[{offset}] != 0', 'u8' : '{buffer}[{offset}]', 'u16' : '(({buffer}[{offset}] << 8) | {buffer}[{offset}+1])',
                      'u32' : '(({buffer}[{offset}] << 24) | ({buffer}[{offset}+1] << 16) | ({buffer}[{offset}+2] << 8) | {buffer}[{offset}+3])',
                      'string' : '(const char *) &{buffer}[{offset}]', 'u8[]' : '&{buffer}[{offset}]'}

c_getter_len_template_zero = '''static inline uint16_t {fn_name}_len(uint16_t payload_len) {{
    return payload_len;
}}
'''
c_getter_len_template = '''static inline uint16_t {fn_name}_len(uint16_t payload_len) {{
    return payload_len - {offset};
}}
'''
c_payload_getter_template = '''static inline {fn_type} {fn_name}(const uint8_t * {argument_name}) {{
    return {accessor};
}}
'''

c_message_getter_template = '''static inline {fn_type} {fn_name}(const mb_message_t * {argument_name}) {{
    return {accessor};
}}
'''

c_is_event_header_template = '''bool {fn_name}(uint8_t opcode);
'''

c_is_event_code_template = '''bool {fn_name}(uint8_t opcode){{
    switch (opcode){{
{switch_body}        default:
            return false;
    }}
}}
'''

c_header_start = '''
#ifndef MULTIBUS_PROTOCOL_H_
#define MULTIBUS_PROTOCOL_H_

// Generated from protocol/multibus.yml

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t         component;
    uint8_t         channel;
    uint8_t         operation;
    uint16_t        payload_len;
    const uint8_t * payload_data;
} mb_message_t;

'''

c_header_end = '''

#if defined __cplusplus
}
#endif

#endif // MULTIBUS_PROTOCOL_H_
'''


c_code_start = '''
// MultiBus Protocol Implementation

// Generated from protocol/multibus.yml

#include "multibus_protocol.h"

'''

c_code_end = '''
'''

c_transport_start = '''
#ifndef MULTIBUS_PROTOCOL_TRANSPORT_H_
#define MULTIBUS_PROTOCOL_TRANSPORT_H_

// Generated from protocol/multibus.yml

#include "multibus_protocol.h"
#include "multibus_transport.h"

#if defined __cplusplus
extern "C" {
#endif

'''

c_transport_end = '''

#if defined __cplusplus
}
#endif

#endif // MULTIBUS_PROTOCOL_TRANSPORT_H_
'''

def c_type_for_enum_name(enum_name):
    return 'mb_' + enum_name.lower() + '_t'

def c_type_for_field(field_name, mb_type):
    if mb_type == 'enum':
        return c_type_for_enum_name(field_name)
    else:
        return c_types[mb_type]


def c_getter_len(fn_name, offset):
    if offset == 0:
        return c_getter_len_template_zero.format(fn_name=fn_name, offset=offset)
    else:
        return c_getter_len_template.format(fn_name=fn_name, offset=offset)

def c_payload_getter(fn_name, field_name, c_type, mb_type, offset, argument_name):
    if mb_type == 'enum':
        accessor = '(%s) ' % c_type + c_buffer_accessor['u8'].format(buffer=argument_name, offset=offset)
    else:
        accessor = c_buffer_accessor[mb_type].format(buffer=argument_name, offset=offset)
    return c_payload_getter_template.format(fn_type=c_type, fn_name=fn_name, accessor=accessor, argument_name=argument_name)

def c_message_getter(fn_name, field_name, c_type, mb_type, offset, argument_name):
    if mb_type == 'enum':
        accessor = '(%s) ' % c_type + c_buffer_accessor['u8'].format(buffer=argument_name+'->payload_data', offset=offset)
    else:
        accessor = c_buffer_accessor[mb_type].format(buffer=argument_name+'->payload_data', offset=offset)
    return c_message_getter_template.format(fn_type=c_type, fn_name=fn_name, accessor=accessor, argument_name=argument_name)

def c_arguments(fields):
    arguments = []
    for (field_name, mb_type) in fields:
        arguments.append((c_type_for_field(field_name, mb_type), field_name))
    return ", ".join(["%s %s" % (field_type, field_name) for (field_type, field_name) in arguments])

def c_write_enum(fout, enum_name, values):
    c_type = c_type_for_enum_name(enum_name)
    fout.write("typedef enum {\n")
    for (name, value) in values.items():
        if type(name) is int:
            name = str(name)
        if type(value) is int:
            value = "0x%x" % value
        fout.write('    MB_%s_%s = %s,\n' % (enum_name.upper(), name.upper(), value))
    fout.write("} %s;\n" % c_type)

def c_generate_header(gen_path):

    with open(gen_path, 'wt') as fout: 

        fout.write(c_header_start)
        fout.write("// MultiBus Protocol Version\n")
        fout.write("#define MB_PROTOCOL_VERSION %s\n" % protocol_version)
        fout.write("\n")

        # calc payload_offset
        payload_offset = 0
        for (field, mb_type) in header.items():
            payload_offset += c_size[mb_type]

        # header size
        fout.write("// MultiBus Protocol Header Size\n")
        fout.write("#define MB_HEADER_SIZE %u\n" % payload_offset)
        fout.write("\n")

        # generate component enum
        fout.write("// Component Enumeration\n")
        component_enum = {}
        for (component_name, component) in components.items():
            name_upper = component_name.upper()
            component_id = component['id']
            component_enum[name_upper] = component_id
        c_write_enum(fout, 'component', component_enum)
        fout.write("\n")

        # generate operation enums
        fout.write("// Operation Enumerations\n")
        for (component_name, component) in components.items():
            operation_enum = {}
            for (operation_name, operation) in component['operations'].items():
                operation_id = operation['id']
                operation_enum[operation_name.upper()] = operation_id
            c_write_enum(fout, 'operation_' + component_name, operation_enum)
            fout.write("\n")

        # generate general enums
        fout.write("// General Enumerations\n")
        for (enum_name, enum_values) in general_enums.items():
            c_write_enum(fout, enum_name, enum_values)
        fout.write("\n")

        # generate field enums
        fout.write("// Field Enumerations\n")
        for (enum_name, enum_values) in field_enums.items():
            c_write_enum(fout, enum_name, enum_values)
        fout.write("\n")

        # generate getters for header fields
        fout.write("// MultiBus Protocol Getter for Header\n")
        offset = 0
        for (field, mb_type) in header.items():
            if mb_type == 'enum':
                c_type = c_type_for_enum_name(field)
            else:
                c_type = c_types[mb_type]
            fout.write(c_payload_getter('mb_header_get_' + field, field, c_type, mb_type, offset, 'header'))
            offset += c_size[mb_type]
        fout.write("\n")

        # generate header builder
        fout.write("// MultiBus Header Builder\n")
        fout.write("void mb_header_setup(uint8_t * buffer, ")
        fout.write(c_arguments(header.items()))
        fout.write(");\n\n")

        # generate getter and builder for each component operation
        for (component_name, component) in components.items():

            # is_event
            fout.write("// Component: %s, is_event\n" % component_name)
            fn_name = "mb_" + component_name + "_is_event"
            fout.write(c_is_event_header_template.format(fn_name=fn_name))
            fout.write('\n')

            for (operation_name, operation) in component['operations'].items():
                fout.write("// Component: %s, Operation: %s\n" % (component_name, operation_name))

                operation_fields = operation['fields']
                if operation_fields is None:
                    operation_fields = {}

                # generate one getter per operation+field
                offset = 0
                name_payload_prefix = "mb_" + component_name + "_" + operation_name + "_get_"
                name_message_prefix = "mb_message_" + component_name + "_" + operation_name + "_get_"
                for (field, mb_type) in operation_fields.items():
                    if type(mb_type) is dict:
                        field = component_name + "_" + operation_name + '_' + field
                        mb_type = 'enum'
                    if mb_type == 'enum':
                        c_type = c_type_for_enum_name(field)
                    else:
                        c_type = c_types[mb_type]
                    if mb_type in ['u8[]','string']:
                        fout.write(c_getter_len(name_payload_prefix + field, offset))
                    # getter using payload
                    fout.write(c_payload_getter(name_payload_prefix + field, field, c_type, mb_type, offset, 'payload'))
                    # getter using message
                    fout.write(c_message_getter(name_message_prefix + field, field, c_type, mb_type, offset, 'message'))
                    offset += c_size[mb_type]
                fout.write("\n")

                # message builder
                fn_name = "mb_" + component_name + "_" + operation_name + "_setup"
                offset = payload_offset
                fields = [('buffer_data', 'buffer'), ('buffer_len','u16'), ('channel','u8') ]
                for (field, mb_type) in operation_fields.items():
                    if type(mb_type) is dict:
                        field = component_name + "_" + operation_name + '_' + field
                        mb_type = 'enum'
                    if mb_type == "u8[]":
                        fields.append((field+"_len", 'u16'))   
                    fields.append((field, mb_type))
                fout.write("uint16_t " + fn_name + "(" + c_arguments(fields) + ");\n")
                fout.write('\n')

        fout.write(c_header_end)

def c_generate_code(gen_path):

    with open(gen_path, 'wt') as fout: 

        fout.write(c_code_start)

        # generate header builder
        fout.write("// MultiBus Header Builder\n")
        fout.write("void mb_header_setup(uint8_t * buffer, ")
        fout.write(c_arguments(header.items()))
        fout.write("){\n")
        offset = 0

        # calc payload_offset
        payload_offset = 0
        for (field, mb_type) in header.items():
            payload_offset += c_size[mb_type]
            if mb_type == 'u8':
                fout.write('    buffer[{offset}] = {field};\n'.format(field=field, offset=offset))
                offset += 1
            elif mb_type == 'u16':
                fout.write('    buffer[{offset}] = {field} >> 8;\n'.format(field=field, offset=offset))
                fout.write('    buffer[{offset}] = {field} & 0xff;\n'.format(field=field, offset=offset+1))
                offset += 2
            elif mb_type == 'enum':
                c_type = c_type_for_enum_name(field)
                fout.write( '    buffer[{offset}] = ({c_type}) {field};\n'.format(field=field, c_type = c_type, offset=offset))
                offset += 1
        fout.write('}\n\n')

        # generate getter and builder for each component operation
        for (component_name, component) in components.items():

            event_ids = []
            for (operation_name, operation) in component['operations'].items():
                fout.write("// Component: %s, Operation: %s\n" % (component_name, operation_name))

                operation_fields = operation['fields']
                if operation_fields is None:
                    operation_fields = {}
                if 'type' in operation:
                    if operation['type'] == 'event':
                        event_ids.append(operation['id'])

                # message builder
                setup_fn_name = "mb_" + component_name + "_" + operation_name + "_setup"
                offset = payload_offset
                fields = [('buffer_data', 'buffer'), ('buffer_len','u16'), ('channel','u8') ]
                for (field, mb_type) in operation_fields.items():
                    if type(mb_type) is dict:
                        field = component_name + "_" + operation_name + '_' + field
                        mb_type = 'enum'
                    if mb_type == "u8[]":
                        fields.append((field+"_len", 'u16'))   
                    fields.append((field, mb_type))
                body = ""
                variable_field_len = None
                string_field = None
                for (field, mb_type) in operation_fields.items():
                    if type(mb_type) is dict:
                        field = component_name + "_" + operation_name + '_' + field
                        c_type = c_type_for_enum_name(field)
                        body += '    buffer_data[{offset}] = ({c_type}) {field};\n'.format(field=field, c_type = c_type, offset=offset)
                        offset += 1
                    elif mb_type == 'bool':
                        body += '    buffer_data[{offset}] = {field} ? 1 : 0;\n'.format(field=field, offset=offset)
                        offset += 1
                    elif mb_type == 'u8':
                        body += '    buffer_data[{offset}] = {field};\n'.format(field=field, offset=offset)
                        offset += 1
                    elif mb_type == 'u16':
                        body += '    buffer_data[{offset}] = {field} >> 8;\n'.format(field=field, offset=offset)
                        body += '    buffer_data[{offset}] = {field} & 0xff;\n'.format(field=field, offset=offset+1)
                        offset += 2
                    elif mb_type == 'u32':
                        body += '    buffer_data[{offset}] = {field} >> 24;\n'.format(field=field, offset=offset)
                        body += '    buffer_data[{offset}] = {field} >> 16;\n'.format(field=field, offset=offset+1)
                        body += '    buffer_data[{offset}] = {field} >> 8;\n'.format(field=field, offset=offset+2)
                        body += '    buffer_data[{offset}] = {field} & 0xff;\n'.format(field=field, offset=offset+3)
                        offset += 4
                    elif mb_type == 'u8[]':
                        variable_field_len = field+'_len'
                        body += '    memcpy(&buffer_data[{offset}], {field}, {field}_len);\n'.format(offset=offset,field=field)
                    elif mb_type == 'string':
                        string_field = field
                        variable_field_len = field+"_len"
                        body += '    memcpy(&buffer_data[{offset}], {field}, {field}_len);\n'.format(offset=offset,field=field)
                    elif mb_type == 'enum':
                        body += '    buffer_data[{offset}] = (uint8_t) {field};\n'.format(field=field, offset=offset)
                        offset += 1
                    else:
                        body += '    /* type %s not handled yet */\n' % mb_type

                fout.write("uint16_t " + setup_fn_name + "(" + c_arguments(fields) + "){\n")
                if string_field is not None:
                    fout.write('    uint16_t {var_len} = strlen({string_field});\n'.format(var_len=variable_field_len,string_field=string_field))
                if variable_field_len is None:
                    fout.write('    uint32_t payload_len = {offset};\n'.format(offset=offset-payload_offset))
                else:
                    fout.write('    uint32_t payload_len = {offset} + {var_len};\n'.format(offset=offset-payload_offset, var_len=variable_field_len))
                fout.write('    uint32_t message_len = payload_len + MB_HEADER_SIZE;\n')
                fout.write("    assert(buffer_len >= message_len);\n")
                fout.write("    mb_header_setup(buffer_data, {component_id}, (uint8_t) {opcode}, channel, payload_len);\n".format(
                    component_id='MB_COMPONENT_' + component_name.upper(), opcode='MB_OPERATION_'+ component_name.upper() + "_" + operation_name.upper()))
                fout.write(body)
                fout.write('    return message_len;\n')
                fout.write("}\n")
                fout.write('\n')

            # is_event
            fn_name = "mb_" + component_name + "_is_event"
            switch_body = "\n".join([("        case %s:\n" % event_id) for event_id in event_ids])
            if len(event_ids) > 0:
                switch_body += "            return true;\n"
            fout.write(c_is_event_code_template.format(fn_name=fn_name,switch_body=switch_body))
            fout.write('\n')


        fout.write(c_code_end)

def c_generate_transport_helper(gen_path):

    with open(gen_path, 'wt') as fout:

        fout.write(c_transport_start)

        # generate send function for each component operation
        for (component_name, component) in components.items():
            for (operation_name, operation) in component['operations'].items():
                fout.write("// Component: %s, Operation: %s\n" % (component_name, operation_name))

                operation_fields = operation['fields']
                if operation_fields is None:
                    operation_fields = {}

                # message builder
                fn_name   = "mb_transport_" + component_name + "_" + operation_name + '_send'
                setup_fn = "mb_" + component_name + "_" + operation_name + '_setup'
                fields = [('channel','u8') ]
                for (field, mb_type) in operation_fields.items():
                    if type(mb_type) is dict:
                        field = component_name + "_" + operation_name + '_' + field
                        mb_type = 'enum'
                    if mb_type == "u8[]":
                        fields.append((field+"_len", 'u16'))
                    fields.append((field, mb_type))
                body = ""
                variable_field_len = None

                fout.write("static inline void " + fn_name + "(mb_transport_t * transport, " + c_arguments(fields) + "){\n")
                fout.write("    uint16_t request_len = " + setup_fn + "(transport->send_buffer_storage, transport->send_buffer_size," + ",".join([name for (name,_) in fields]) + ');\n')
                fout.write("    mb_transport_send(transport, transport->send_buffer_storage, request_len);\n")
                fout.write("}\n")
                fout.write('\n')

        fout.write(c_transport_end)

# main

## get paths
multibus_root = os.path.abspath(os.path.dirname(sys.argv[0]) + '/..')
protocol_path = multibus_root+'/protocol/multibus.yml'

# use argument as path if given
if (len(sys.argv) < 2):
    gen_path = multibus_root + '/protocol'
else:
    gen_path = sys.argv[1]

c_generate_header_path    = gen_path + "/multibus_protocol.h"
c_generate_code_path      = gen_path + "/multibus_protocol.c"
c_generate_transport_path = gen_path + '/multibus_transport_protocol.h'

result = parser.load_protocol_description(protocol_path)

protocol_version = result['protocol_version']
header           = result['header']
components       = result['components']
general_enums    = result['general_enums']
field_enums      = result['field_enums']

c_generate_header(c_generate_header_path)
c_generate_code(c_generate_code_path)
c_generate_transport_helper(c_generate_transport_path)
