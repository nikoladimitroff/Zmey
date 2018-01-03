import os
from os import path
import re
from caseinsensitive_dict import CaseInsensitiveDict

class Helpers:
    REFLECTED_REGEX = (
            r"\[\[" # opening [[
                r"Reflected" # the keyword Reflected
                # The argument group which might be missing. Matches either a single arg or a multiple comma separated args
                # Each args must be one of:
                # 1. Foo = number
                # 2. Foo = "string"
                # 3. Foo = { "initializer", "string" }
                r"(?:\(("
                    r"?P<arguments>" # name the arguments group
                        # Matches repeating sequences of comma separated Key=Value pairs
                        r"((\w+(\s*=\s*(?:\d+|\".*\"|\{.*\}))?),\s*)*"
                        # Matches just one, optional comma separated Key=Value pair
                        r"(\w+(\s*=\s*(?:\d+|\".*\"|\{.*\}))?)"
                r")\))?" # the entire group is optional
            r"\]\]" # closing ]]
    )

    REFLECTED_PROPERTY = re.compile(
            REFLECTED_REGEX +
            r"\s*(?P<type>[\w:><]+)\s+(?P<name>\w+)")

    REFLECTED_TYPE = re.compile(
            # Matches any class or struct with the Reflected attributed, ends in [\s\S]*? to work with inheritance
            r"(class|struct)\s*" + REFLECTED_REGEX + r"\s*(?P<typename>\w+)[\s\S]*?"
            r"\{"
                r"(?P<typebody>[\s\S]*)"
            r"\};")

    @staticmethod
    def parse_reflected_arguments(regex_match):
        raw_text = regex_match.group("arguments") \
                   if "arguments" in regex_match.re.groupindex \
                   else ""
        result = CaseInsensitiveDict()
        if not raw_text:
            return result
        # TODO: This won't work for initializer lists
        # It needs to be non-regular!
        for key_value_pair in raw_text.split(","):
            if "=" in key_value_pair:
                (key, value) = (text.strip() for text in key_value_pair.split("="))
            else:
                (key, value) = (key_value_pair, None)
            result[key] = value
        return result


class ReflectedProperty:
    def __init__(self, regex_match):
        # regex_match is a match over Helpers.REFLECTED_PROPERTY
        self.typename = regex_match.group("type")
        self.name = regex_match.group("name")
        reflected_args = Helpers.parse_reflected_arguments(regex_match)
        self.default_value = reflected_args.get("Default")

    def __str__(self):
        return f"{self.typename} {self.name};"


class ReflectedType:
    def __init__(self, regex_match):
        self.typename = regex_match.group("typename")
        print( regex_match.group("typebody"))
        property_matches = re.finditer(Helpers.REFLECTED_PROPERTY, regex_match.group("typebody"))
        self.properties = [ReflectedProperty(match) for match in property_matches]

    def __str__(self):
        indented_props = "".join("\n\t" + str(prop) for prop in self.properties)
        return f"type {self.typename}\n{indented_props}"


class Mirror:
    def __init__(self):
        self.types = []

    def reflect_file(self, cpp_file):
        with open(cpp_file) as code:
            type_patterns = re.finditer(Helpers.REFLECTED_TYPE, code.read())
        processed_patterns = "\n".join(str(ReflectedType(match)) for match in type_patterns)
        if processed_patterns.strip():
           print(f"Found types: \n{processed_patterns}")

    def reflect_directory(self, directory):
        for (subdir, _, filelist) in os.walk(directory):
            for file in filelist:
                cpp_file = path.abspath(path.join(subdir, file))
                self.reflect_file(cpp_file)


if __name__ == "__main__":
    mirror = Mirror()
    mirror.reflect_directory("E:\\Dev\\ReserveZmey\\include\\Zmey\\Components")
