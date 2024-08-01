# better-copy - File Copy Utility

---

## Overview

better-copy is a simple command-line utility for copying files in Windows. The utility displays the progress of the copy operation in real-time.

## Features

- Copy files with progress indication.
- Verify the integrity of the copied file.
- Options to overwrite or skip existing files.
- User-friendly help menu.

## Usage

To use bcopy.exe, open a command prompt and run the following command and you will get a help menu:

```
bcopy.exe /?
```

#### Options

- /v : Verify the copy after completion.
- /y : Overwrite the destination file without asking.
- /n : Skip copying if the destination file exists.
- /? : Show the help menu.

#### Parameters

- source : The path to the source file you want to copy.
- destination : The path where the file should be copied. If not provided, the file will be copied to the current directory. If it is a directory, it should end with a double backslash (e.g., E:\\example\\New folder\\\\).

## Notes

- If the destination file already exists, you will be prompted to overwrite it unless the `/y` or `/n` options are used.

- This is a very simple utility and is provided as-is, Use it at your own risk.
