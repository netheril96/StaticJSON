#!/usr/bin/env python

from jsonschema import validate, ValidationError
import json


def main():
    with open('user_array_schema.json', 'rb') as f:
        user_array_schema = json.load(f)
    with open('user_map_schema.json', 'rb') as f:
        user_map_schema = json.load(f)

    for fn in ('../examples/success/user_array.json', '../examples/success/user_array_compact.json'):
        with open(fn, 'rb') as f:
            data = json.load(f)
        validate(data, user_array_schema)

    for fn in ('../examples/success/user_map.json',):
        with open(fn, 'rb') as f:
            data = json.load(f)
        validate(data, user_map_schema)

    for fn in ['out_of_range.json', 'integer_string.json', 'missing_required.json', 'map_element_mismatch.json',
               'null_in_key.json', 'single_object.json', 'unknown_field.json']:
        with open('../examples/failure/' + fn, 'rb') as f:
            data = json.load(f)
        try:
            validate(data, user_array_schema)
        except ValidationError:
            pass
        else:
            raise AssertionError("Validation should fail!")
        try:
            validate(data, user_map_schema)
        except ValidationError:
            pass
        else:
            raise AssertionError("Validation should fail!")


if __name__ == '__main__':
    main()
