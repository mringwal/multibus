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

# import YAML library with warning
# https://pyyaml.org/wiki/PyYAMLDocumentation
try:
    import yaml
except ImportError as error:
        print ("Please install yaml with '%s -m pip install pyyaml'\n\n" % sys.executable)
        sys.exit(10)

program_info = '''Bindings Generator for MultiBus
Copyright 2022, MultiBus
'''

## Collected data
protocol_version = 0
header = {}
components = {}
general_enums = {}
field_enums = {}

## process protocol description
def process_protocol_description(data):
    global protocol_version
    global components
    global header
    global general_enums
    global field_enums

    # get protocol version
    protocol_version = data['version']

    # get message header fields
    message = data['message']
    header = message['fields']

    # get components
    components = message['components']

    # collect general and field enums
    for (enum_name, enum) in message['enums'].items():
        values = {}
        for (value_name, value) in enum.items():
            values[value_name] = value
        general_enums[enum_name] = values
    for (component_name, component) in components.items():
        if 'enums' in component:
            for (enum_name, enum) in component['enums'].items():
                values = {}
                if enum_name in general_enums:
                    values = general_enums[enum_name]
                for (value_name, value) in enum.items():
                    qualified_value_name = component_name + '_' + value_name
                    values[qualified_value_name] = value
                general_enums[enum_name] = values
        for (operation_name, operation) in component['operations'].items():
            operation_fields = operation['fields']
            if operation_fields is None:
                continue
            for (field_name, field) in operation_fields.items():
                if type(field) is dict:
                    values = {}
                    qualified_field_name = "_".join([component_name, operation_name, field_name])
                    for (value_name, value) in field.items():
                        value_string = '0x%x' % value
                        values[value_name] = value_string
                    field_enums[qualified_field_name] = values

def load_protocol_description(protocol_path):
    result = {}
    with open(protocol_path, "r") as stream:
        try:
            process_protocol_description(yaml.safe_load(stream))
            result['protocol_version'] = protocol_version
            result['header'] = header
            result['components'] = components
            result['general_enums'] = general_enums
            result['field_enums'] = field_enums
        except yaml.YAMLError as exc:
            print(exc)
    return result

