"""
Test modules and libraries
"""

import gzip
import argparse
import json
import base64
import multiprocessing
import glob


def _check_gzip() -> bool:
    gzip.compress(b"test")
    return True


def _check_argparse() -> bool:
    parser = argparse.ArgumentParser()
    parser.add_argument("--test", type=str, help="Test argument")
    args = parser.parse_args(["--test", "value"])
    return args.test == "value"


def _check_json() -> bool:
    data = {"key": "value"}
    json_str = json.dumps(data)
    loaded_data = json.loads(json_str)
    return loaded_data == data


def _check_base64() -> bool:
    original = b"test"
    encoded = base64.b64encode(original)
    decoded = base64.b64decode(encoded)
    return decoded == original


def _map_func(x: int):
    return x * x


def _check_multiprocessing() -> bool:
    test_input = [1, 2, 3, 4, 5]
    result = multiprocessing.Pool().map(_map_func, test_input)
    return result == [1, 4, 9, 16, 25]


def _check_glob() -> bool:
    files = glob.glob("*.py")
    return isinstance(files, list)


def check_all() -> bool:
    checks = [
        _check_gzip,
        _check_argparse,
        _check_json,
        _check_base64,
        _check_multiprocessing,
        _check_glob,
    ]
    check_result = [check() for check in checks]
    return all(check_result)


if __name__ == "__main__":
    try:
        if check_all():
            print("1")
        else:
            print("0")
    except Exception as e:
        print(f"0")
