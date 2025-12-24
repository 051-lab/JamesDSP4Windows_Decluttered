import os
import re

def analyze_eel_files(directory):
    signatures = {}
    
    # Heuristic regex for function calls: name(
    call_pattern = re.compile(r'([a-zA-Z0-9_]+)\s*\(')

    print(f"Scanning directory: {directory}")
    
    try:
        files = [f for f in os.listdir(directory) if f.endswith(".eel")]
    except Exception as e:
        print(f"Error listing directory: {e}")
        return {}

    for file in files:
        filepath = os.path.join(directory, file)
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {file}: {e}")
            continue
            
        # Remove comments
        content = re.sub(r'//.*', '', content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        # Find all potential function calls
        for match in call_pattern.finditer(content):
            func_name = match.group(1)
            # Skip common keywords and confirmed OK functions for noise reduction if needed
            if func_name in ['function', 'if', 'loop', 'while']: continue 
            
            # Parse args
            start_idx = match.end()
            balance = 1
            current_arg_count = 1
            has_args = False
            
            # Check if empty ( )
            # Fast forward whitespace
            idx = start_idx
            while idx < len(content) and content[idx].isspace():
                idx += 1
            
            if idx < len(content) and content[idx] == ')':
                    current_arg_count = 0
                    balance = 0
            else:
                for i in range(start_idx, len(content)):
                    char = content[i]
                    if char == '(':
                        balance += 1
                    elif char == ')':
                        balance -= 1
                        if balance == 0:
                            break
                    elif char == ',' and balance == 1:
                        current_arg_count += 1
                    elif not char.isspace():
                        has_args = True
                
                if not has_args and current_arg_count == 1:
                    current_arg_count = 0

            if func_name not in signatures:
                signatures[func_name] = set()
            signatures[func_name].add(current_arg_count)

    return signatures

results = analyze_eel_files(r"C:\Users\gmaym\.gemini\antigravity\playground\blazing-comet\JamesDSP-Windows\assets\Liveprog")

with open("eel_report.txt", "w") as f:
    for func, counts in sorted(results.items()):
        f.write(f"{func}: {sorted(list(counts))}\n")
        
print("Analysis complete. Results written to eel_report.txt")
