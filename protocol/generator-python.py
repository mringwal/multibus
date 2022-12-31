#!/usr/bin/env python3

# Copyright 2022 Boris Zweimueller
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

# Collected data
protocol_version = 0
header = {}
components = {}
general_enums = {}
field_enums = {}

protocol_type_sizes = {'bool': 1, 'u8': 1, 'u16': 2, 'u32': 4, 'string': 0, 'u8[]': 0, 'enum': 1}

py_is_event_code_template = '''def {fn_name}(opcode):
    return False

'''

py_header_start = '''
import struct

'''


def py_arguments(fields):
    return ", ".join(["%s" % arg[0] for arg in fields])


def generate_python_bindings(py_generate_header_path):
    with open(py_generate_header_path, 'wt') as out:
        out.write("# Generated from protocol/multibus.yml\n\n")
        out.write(py_header_start)
        out.write("# MultiBus Protocol Version\n")
        out.write("MB_PROTOCOL_VERSION = %s\n" % protocol_version)

        generate_enums_and_constants(out)

        payload_offset = get_payload_offset()

        generate_header_setup_functions(out, payload_offset)

        # Build request setup methods and response getters
        for (component_name, component) in components.items():

            # is_event
            fn_name = "mb_" + component_name + "_is_event"
            out.write(py_is_event_code_template.format(fn_name=fn_name))
            out.write('\n')

            for (operation_name, operation) in component['operations'].items():
                out.write("# Component: %s, Operation: %s\n" % (component_name, operation_name))

                operation_fields = operation['fields']
                if operation_fields is None:
                    operation_fields = {}

                if "response" in operation_name:
                    create_getters_for_response(component_name, operation_fields, operation_name, out)

                if "request" in operation_name:
                    create_request_message_functions(component, component_name, operation, operation_fields,
                                                     operation_name, out, payload_offset)


def create_request_message_functions(component, component_name, operation, operation_fields, operation_name, out,
                                     payload_offset):
    # message builder
    setup_fn_name = "mb_" + component_name + "_" + operation_name + "_setup"
    offset = payload_offset
    # arguments for message setup functions. 'channel' is always there.
    arguments = [('channel', 'u8')]
    for (field, mb_type) in operation_fields.items():
        if type(mb_type) is dict:
            arguments.append((field, 'u8'))  # simply use an u8 for enum - should be enough
        elif mb_type == "u8[]":
            arguments.append((field + "_len", 'u16'))
            arguments.append((field, mb_type))
        else:
            arguments.append((field, mb_type))

    body = ""
    variable_field_len = None
    pack_string = '>'
    for (field, mb_type) in operation_fields.items():
        if type(mb_type) is dict:
            # simply use an u8 for enum - should be enough
            pack_string += 'B'
            offset += 1
            continue
        if mb_type == 'u8':
            pack_string += 'B'
            offset += 1
        elif mb_type == 'bool':
            pack_string += 'B'
            offset += 1
        elif mb_type == 'u16':
            pack_string += 'H'
            offset += 2
        elif mb_type == 'u32':
            pack_string += 'I'
            offset += 4
        elif mb_type == 'u8[]':
            variable_field_len = field + '_len'
            pack_string += "' + str(data_len) + 's"
        else:
            body += '    # type %s not handled yet\n' % mb_type

    body += "    payload = struct.pack('" + pack_string + "', " + ', '.join(
        operation_fields.keys()) + ")\n"
    out.write("def " + setup_fn_name + "(" + py_arguments(arguments) + "):\n")
    if variable_field_len is None:
        out.write('    payload_len = {offset}\n'.format(offset=offset - payload_offset))
    else:
        out.write('    payload_len = {offset} + {var_len}\n'.format(offset=offset - payload_offset,
                                                                    var_len=variable_field_len))
    out.write("    header = mb_header_setup({component_id}, {opcode}, channel, payload_len)\n".format(
        component_id=component['id'], opcode=operation['id']))
    if len(operation_fields) == 0:
        out.write('    return header\n')
    else:
        out.write(body)
        out.write('    return header + payload\n')
    out.write("\n")
    out.write('\n')


def create_getters_for_response(component_name, operation_fields, operation_name, out):
    pack_string = '>'
    if len(operation_fields) == 0:
        body = 'pass'
    else:
        pack_len = 0
        for (field, mb_type) in operation_fields.items():
            if mb_type == 'enum':
                pack_string += 'B'
                pack_len += 1
            elif mb_type == 'u8':
                pack_string += 'B'
                pack_len += 1
            elif mb_type == 'bool':
                pack_string += 'B'
                pack_len += 1
            elif mb_type == 'u16':
                pack_string += 'H'
                pack_len += 2
            elif mb_type == 'u32':
                pack_string += 'I'
                pack_len += 4
            elif mb_type == 'u8[]':
                pack_string += "' + str(len(payload) - " + str(pack_len) + ") + 's"
            elif mb_type == 'string':
                pack_string += "' + str(len(payload) - " + str(pack_len) + ") + 's"
            else:
                raise SystemError("Not handeled type detected: ", mb_type)

        body = ', '.join(operation_fields.keys()) + " = struct.unpack('" + pack_string + "', payload)\n"
        body += "    return " + ', '.join(operation_fields.keys())
        if len(operation_fields) == 1:
            body += "[0]"
    out.write("def mb_" + component_name + "_" + operation_name + "(payload):\n")
    out.write("    " + body)
    out.write("\n\n\n")


def generate_header_setup_functions(out, payload_offset):
    # header size
    out.write("# MultiBus Protocol Header Size\n")
    out.write("MB_HEADER_SIZE = %u\n" % payload_offset)
    out.write("\n")
    # generate header builder
    out.write("\n# MultiBus Header Builder\n")
    out.write("def mb_header_setup(")
    out.write(', '.join(header.keys()))
    out.write("): \n")
    pack_string = '>'
    for (mb_type) in header.values():
        if mb_type == 'u8':
            pack_string += 'B'
        elif mb_type == 'bool':
            pack_string += 'B'
        elif mb_type == 'enum':
            pack_string += 'B'
        elif mb_type == 'u16':
            pack_string += 'H'
    out.write("    return struct.pack('" + pack_string + "', " + ', '.join(header.keys()) + ")\n")
    out.write('\n\n')


def get_payload_offset() -> int:
    payload_offset = 0
    for (field, mb_type) in header.items():
        payload_offset += protocol_type_sizes[mb_type]
    return payload_offset


def generate_enums_and_constants(out):
    # generate component enum
    out.write("\n")
    out.write("# Component Enumeration\n")
    for (component_name, component) in components.items():
        name_upper = component_name.upper()
        out.write("MB_COMPONENT_ID_%s = 0x%02x\n" % (name_upper, component['id']))
    out.write("\n")
    # generate operation enums
    out.write("# Operation Enumeration\n")
    for (component_name, component) in components.items():
        component_name_upper = component_name.upper()
        for (operation_name, operation) in component['operations'].items():
            out.write("MB_OPERATION_ID_%s_%s = 0x%02x\n" % (
                component_name_upper, operation_name.upper(), operation['id']))
    out.write("\n")
    # generate general enums
    out.write("# General Enumerations\n")
    for (enum_name, enum_values) in general_enums.items():
        for k, v in enum_values.items():
            out.write("MB_" + enum_name.upper() + "_" + k.upper() + " = 0x%02x\n" % v)
    out.write("\n")
    # generate field enums
    out.write("# Field Enumerations\n")
    for (enum_name, enum_values) in field_enums.items():
        for k, v in enum_values.items():
            out.write("MB_" + enum_name.upper() + "_" + k.upper() + " = %s\n" % v)
    out.write("\n")


def main():
    global protocol_version, header, components, general_enums, field_enums

    multibus_root = os.path.abspath(os.path.dirname(sys.argv[0]) + '/..')
    protocol_path = multibus_root + '/protocol/multibus.yml'

    # use argument as path if given
    if len(sys.argv) < 2:
        gen_path = multibus_root + '/protocol'
    else:
        gen_path = sys.argv[1]

    py_generate_header_path = gen_path + "/multibus_protocol.py"

    result = parser.load_protocol_description(protocol_path)

    protocol_version = result['protocol_version']
    header = result['header']
    components = result['components']
    general_enums = result['general_enums']
    field_enums = result['field_enums']

    generate_python_bindings(py_generate_header_path)


if __name__ == "__main__":
    main()
