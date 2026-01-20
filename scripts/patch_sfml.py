import os
import re

file_path = 'cmake/Macros.cmake'

def patch():
    if not os.path.exists(file_path):
        print(f"File {file_path} not found.")
        exit(1)

    with open(file_path, 'r') as f:
        content = f.read()

    target_block = '    if (TARGET ${target})\n        return()\n    endif()'
    
    if 'return()' in content and 'TARGET ${target}' in content:
        print(f"{file_path} is already patched.")
        return

    old_block = '    if (TARGET ${target})\n        message(FATAL_ERROR "Target \'${target}\' is already defined")\n    endif()'

    if old_block in content:
        print(f"Patching {file_path}...")
        new_content = content.replace(old_block, target_block)
        with open(file_path, 'w') as f:
            f.write(new_content)
        print("Success!")
    else:
        pattern = re.compile(r'if\s*\(TARGET\s*\$\{target\}\)\s*message\(FATAL_ERROR\s*"Target\s*\'\$\{target\}\'\s*is\s*already\s*defined"\)\s*endif\(\)', re.MULTILINE)
        if pattern.search(content):
            print(f"Patching {file_path} with regex...")
            new_content = pattern.sub(target_block, content)
            with open(file_path, 'w') as f:
                f.write(new_content)
            print("Success (Regex)!")
        else:
            print("Could not find patterns to patch. SFML might have changed or is already patched.")
            if 'TARGET ${target}' in content:
                 print("Found 'TARGET ${target}', but not the fatal error message. SFML might be already patched or different.")
            else:
                 exit(1)

if __name__ == "__main__":
    patch()
