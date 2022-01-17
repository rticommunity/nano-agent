#!/usr/bin/env python
"""
Script to embed a file into a header file.

(c) Copyright, Real-Time Innovations, Inc. 2018-2022.
RTI grants Licensee a license to use, modify, compile, and create derivative
works of the Software solely in combination with RTI Connext DDS. Licensee
may redistribute copies of the Software provided that all such copies are
subject to this License. The Software is provided "as is", with no warranty
of any type, including any warranty for fitness for any purpose. RTI is
under no obligation to maintain or support the Software. RTI shall not be
liable for any incidental or consequential damages arising out of the use or
inability to use the Software. For purposes of clarity, nothing in this
License prevents Licensee from using alternate versions of DDS, provided
that Licensee may not combine or link such alternate versions of DDS with
the Software.
"""

import sys
import os

import argparse
from jinja2 import Environment

# Template for header files in jinja2 format
header_template = \
    """
/**
{{ license_text | default("") }}
*/
#include <string.h>

#define {{ str_size_var }} {{ str_size }}
#define {{ str_total_size_var }} {{ str_total_size }}

const char *{{ var }}[{{ str_size_var }}] = {
{%- for s in datacontent %}
    "{{ c_escape(s) }}"{{ "," if not loop.last }}
{%- endfor %}
};

#define {{ var }}_asString(str) {           \\
    int i = 0;                              \\
    (str)[0] = '\\0';                       \\
    for(i = 0; i < {{str_size_var}}; ++i) { \\
        strcat(str, {{var}}[i]);            \\
    }                                       \\
}

"""

def string_escape_c(s):
    return s.replace('\\', '\\\\').replace('\n', "\\n").replace('\"', '\\\"')


def chunk_string(s, chunksize):
    return [s[i:i + chunksize] for i in range(0, len(s), chunksize)]


def embed_into_header(infile, licfile, varname, chunksize, modifier=None):
    with open(infile, encoding='utf-8') as f:
        data = f.read()
    with open(licfile, encoding='utf-8') as f:
        data_lic = f.read()

    datastrarr = chunk_string(data, chunksize)

    ctx = {}
    ctx["str_size_var"] = "{v}_SIZE".format(v=varname)
    ctx["str_size"] = len(datastrarr)
    ctx["str_total_size_var"] = "{v}_TOTAL_SIZE".format(v=varname)
    # We sum up the length of all the substrings in the array of strings
    # Note: We need extra character for \0
    ctx["str_total_size"] = sum(map(len, datastrarr)) + 1
    ctx["var"] = varname
    ctx["var_size"] = len(datastrarr)
    ctx["datacontent"] = datastrarr
    ctx["modifier"] = modifier
    ctx["c_escape"] = string_escape_c
    ctx["license_text"] = data_lic

    return Environment().from_string(header_template).render(**ctx)


def main():
    parser = argparse.ArgumentParser(
        description='Embed a file into a C header file')
    parser.add_argument('-i', '--input')
    parser.add_argument('-l', '--license')
    parser.add_argument('-v', '--varname',
                        help='Variable name to store the string')
    parser.add_argument('-s', '--chunksize', default=1024,
                        help='Split the string in fixed size chunks (default 1024 bytes)')
    parser.add_argument('-k', '--modifier', default=None,
                        help="Visibility modifier (Public, Peer or Private)")
    parser.add_argument('-o', '--output', )
    args = parser.parse_args()

    output = embed_into_header(
        args.input, args.license, args.varname, args.chunksize, args.modifier)

    if not args.output:
        sys.stdout.write(output)
    else:
        # Create the parent dir if it doesn't exist
        parent_dir = os.path.dirname(args.output)
        if not os.path.exists(parent_dir):
            os.makedirs(parent_dir)

        with open(args.output, "w", encoding='utf-8') as f:
            f.write(output)


if __name__ == '__main__':
    main()
