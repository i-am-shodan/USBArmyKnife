import gzip
import os

staticFiles = {}
headerFiles = []

Import("env")

def skip_esp32_libs_from_pi_builds(node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
     to ignore file from a build process, just return None
    """

    if ("usb-ncm" in node.get_path() or "ESP32Marauder" in node.get_path()) and "raspberrypi" in env.GetProjectOption("platform"):
        # Return None for exclude
        print ("Ignoring /lib dir as platform is raspberrypi")
        return None

    return node

# Register callback
env.AddBuildMiddleware(skip_esp32_libs_from_pi_builds, "*")

def compress_to_c_array(srcFile, dstFile, varName, dataConvertFunc=None):
    try:
        # Read the contents of the source file
        with open(srcFile, 'rb') as f:
            data = f.read()

        if dataConvertFunc:
            data = dataConvertFunc(data)

        # Compress the data using gzip
        compressed_data = gzip.compress(data)

        # Convert the compressed data to a C-style array string
        c_array_string = ', '.join(str(byte) for byte in compressed_data)

         # Create the required directories for the destination file
        os.makedirs(os.path.dirname(dstFile), exist_ok=True)

        # Write the C-style array string to the destination file
        try:
            os.remove(dstFile)
        except:
            pass
        with open(dstFile, 'w') as f:
            f.write(f"#pragma once\n\n#ifndef NO_WEB\n\nconst uint8_t PROGMEM {varName}[{len(compressed_data)}] = {{ {c_array_string} }};\n\n#endif")

        print(f"Compressed data written to {dstFile}")

        headerFiles.append(dstFile)
        staticFiles[srcFile.replace("ui/", "/")] = varName
    except FileNotFoundError as e:
        print("Error: Source file not found.")
        print (e)

compress_to_c_array(
    "ui/content/assets/bootstrap/css/bootstrap.min.css", 
    "src/html/bootstrapMinCss.h", 
    "bootstrapMinCssGz",
    lambda data: data.decode('utf-8')
        .replace("@import url(https://fonts.googleapis.com/css?family=Nunito:200,200i,300,300i,400,400i,600,600i,700,700i,800,800i,900,900i&display=swap);", "")
        .replace('"Nunito",', '')
        .encode('utf-8')
)

compress_to_c_array("ui/content/assets/bootstrap/js/bootstrap.min.js", "src/html/bootstrapMinJs.h", "bootstrapMinJsGz")
compress_to_c_array("ui/content/assets/css/styles.min.css", "src/html/stylesMinCss.h", "stylesMinCssGz")
compress_to_c_array("ui/content/assets/js/script.min.js", "src/html/scriptMinJs.h", "scriptMinJsGz")
compress_to_c_array(
    "ui/content/index.html", 
    "src/html/indexHtml.h", "indexHtmlGz", 
    lambda data: data.decode('utf-8').replace("<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css?family=Nunito:200,200i,300,300i,400,400i,600,600i,700,700i,800,800i,900,900i&amp;display=swap\">", "").encode('utf-8')
    )

# Define the directory path
directory_path = "ui/vnc"

# Recursively loop through every file in the directory
for root, dirs, files in os.walk(directory_path):
    for filename in files:
        # Get the full path to the file
        file_path = os.path.join(root, filename).replace("\\", "/")
        
        # Extract the file name without extension
        file_name = filename.replace(".", "").replace("-", "_")

        # Call the compress_to_c_array function
        compress_to_c_array(file_path, f"src/html/vnc/{file_name}.h", f"noVNC{file_name}Gz")

cpp_vector_code = "#ifndef NO_WEB\n\n#include <string>\n#include <unordered_map>\n#include <cstdint>\n#include <pgmspace.h>\n\n";
for header in headerFiles:
    cpp_vector_code += '#include "'+header.replace("src/html/", "")+'"\n'

cpp_vector_code += "\nstd::unordered_map<const char*,std::pair<const uint8_t*, size_t>> staticHtmlFilesLookup = {\n"
lastFile = list(staticFiles.keys())[-1]
for key, value in staticFiles.items():
    path = key.replace("/content/", "/");
    cpp_vector_code += f"    {{\"{path}\",{{ {value},sizeof({value}) }} }}"
    if key != lastFile:
        cpp_vector_code += ",\n"
    else:
        cpp_vector_code += "\n"
cpp_vector_code += "};\n\n#endif"

with open("src/html/htmlFiles.cpp", "w") as file:
    file.write(cpp_vector_code)