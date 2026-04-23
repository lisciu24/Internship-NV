# Build instructions
```bash
cmake -B build .
cmake --build build/
```

# Run instructions
```bash
./build/LogAnalyzer <filepath> <filter>=<value> [<filter>=<value>...]
```

## Available filters:
| filter    | description |
| ---       | --- | 
| From      | Entries after specified date and time [format as in task description]. |
| To        | Entries before specified date and time [format as in task description].  |
| LogLevel  | Entries with specified log level [2nd column]. |
| Source    | Entries with specified source [3rd column]. |
| Message   | Entires that are containing specified message at least partially.|

You can freely combine all of those filters, only one per category.

## Example 

### Input file [input.txt]
```
[2023-10-25T10:00:00] [INFO] [AuthService] User logged in successfully
[2023-10-25T10:05:12] [ERROR] [Database] Connection timeout
[2023-10-25T10:15:30] [WARN] [AuthService] Multiple failed login attempts
[2023-10-25T10:20:00] [ERROR] [Payment] Transaction rejected: insufficient funds
[2023-10-25T10:25:00] [INFO] [Payment] Transaction 12345 processed
```
### Command and output
```
./build/LogAnalyzer input.txt To=2023-10-25T10:16:00 Log_Level=INFO

[2023-10-25T10:00:00] [INFO] [AuthService] User logged in successfully
```

