import argparse


def main():

    parser = argparse.ArgumentParser(description="Convert binary to C++ source/header")

    parser.add_argument(
        "--input", help="Input File", required=True, type=str, dest="input"
    )
    parser.add_argument(
        "--output",
        help="Output Path",
        required=True,
        type=str,
        dest="output",
    )

    parser.add_argument(
        "--type",
        help="Output type: header/source",
        required=True,
        type=str,
        dest="type",
    )

    parser.add_argument(
        "--namespace",
        help="C++ Namespace",
        required=True,
        type=str,
        dest="namespace",
    )

    parser.add_argument(
        "--varname",
        help="C++ Variable Name",
        required=True,
        type=str,
        dest="varname",
    )

    args = parser.parse_args()

    input_path = args.input
    output_path = args.output
    type = args.type
    namespace = args.namespace
    varname = args.varname

    namespace_split = namespace.split("::")
    namespace_heading = "".join([f"namespace {ns} {{ " for ns in namespace_split])
    namespace_trailing = "}" * len(namespace_split)

    if type == "header":
        header_result = "\n".join(
            [
                "// Auto-generated file, do not modify",
                f"// Source: {input_path}",
                "#pragma once",
                "#include <span>",
                "#include <cstddef>",
                namespace_heading,
                f"    // Binary data from {input_path}",
                f"    extern const std::span<const std::byte> {varname};",
                namespace_trailing,
            ]
        )

        with open(output_path, "w") as f:
            f.write(header_result)
        return

    if type == "source":

        with open(input_path, "rb") as f:
            binary_data = f.read()

        binary_size = len(binary_data)
        binary_string = ", ".join(f"0x{byte:02x}" for byte in binary_data)

        source_result = "\n".join(
            [
                "// Auto-generated file, do not modify",
                f"// Source: {input_path}",
                "#include <array>",
                "#include <cstddef>",
                "#include <cstdint>",
                "#include <span>",
                namespace_heading,
                f"    static const std::array<uint8_t, {binary_size}> {varname}_array = {{ {binary_string} }};",
                f"    extern const std::span<const std::byte> {varname} = std::as_bytes(std::span({varname}_array));",
                namespace_trailing,
            ]
        )

        with open(output_path, "w") as f:
            f.write(source_result)
        return

    raise ValueError(f"Unknown type: {type}")


main()
