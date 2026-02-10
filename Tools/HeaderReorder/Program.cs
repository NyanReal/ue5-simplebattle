using System.Text.RegularExpressions;
using HeaderReorder.Services;

// Parse command line arguments
var arguments = args;

if (arguments.Length == 0 || arguments.Contains("--help") || arguments.Contains("-h"))
{
    ShowHelp();
    return 0;
}

// Parse options
string projectPath = "";
bool processAll = false;
int minutes = 10;
bool dryRun = false;
bool verbose = false;

for (int i = 0; i < arguments.Length; i++)
{
    switch (arguments[i])
    {
        case "--all" or "-a":
            processAll = true;
            break;
        case "--minutes" or "-m":
            if (i + 1 < arguments.Length && int.TryParse(arguments[i + 1], out int m))
            {
                minutes = m;
                i++;
            }
            break;
        case "--dry-run" or "-d":
            dryRun = true;
            break;
        case "--verbose" or "-v":
            verbose = true;
            break;
        default:
            if (!arguments[i].StartsWith("-") && string.IsNullOrEmpty(projectPath))
            {
                projectPath = arguments[i];
            }
            break;
    }
}

if (string.IsNullOrEmpty(projectPath))
{
    Console.WriteLine("Error: Project path is required.");
    ShowHelp();
    return 1;
}

// Validate UE5 project
if (!UE5ProjectScanner.IsValidUE5Project(projectPath))
{
    Console.WriteLine($"Error: '{projectPath}' is not a valid UE5 project (no .uproject file found).");
    return 1;
}

Console.WriteLine($"=== UE5 Header Reorder Tool ===");
Console.WriteLine($"Project: {projectPath}");
Console.WriteLine($"Mode: {(processAll ? "All files" : $"Last {minutes} minutes")}");
Console.WriteLine($"Dry-run: {dryRun}");
Console.WriteLine();

// Scan for header files
var scanner = new UE5ProjectScanner(projectPath);
var cutoffTime = processAll ? DateTime.MinValue : DateTime.Now.AddMinutes(-minutes);
var headerFiles = scanner.ScanHeaderFiles(cutoffTime);

Console.WriteLine($"Found {headerFiles.Count} header file(s) to process.");
Console.WriteLine();

if (headerFiles.Count == 0)
{
    Console.WriteLine("No files to process.");
    return 0;
}

// Process each file
var reorderer = new HeaderReorderer();
int modifiedCount = 0;

foreach (var file in headerFiles)
{
    if (verbose)
    {
        Console.WriteLine($"Checking: {file}");
    }

    var result = reorderer.ProcessFile(file, dryRun);
    
    if (result.WasModified)
    {
        modifiedCount++;
        Console.WriteLine($"[{(dryRun ? "WOULD FIX" : "FIXED")}] {Path.GetFileName(file)}");
        
        if (verbose)
        {
            Console.WriteLine($"  Moved: {result.MovedInclude}");
        }
    }
}

Console.WriteLine();
Console.WriteLine($"=== Summary ===");
Console.WriteLine($"Total files checked: {headerFiles.Count}");
Console.WriteLine($"Files {(dryRun ? "needing fix" : "fixed")}: {modifiedCount}");

return 0;

void ShowHelp()
{
    Console.WriteLine("UE5 Header Reorder Tool - Moves .generated.h includes to the end");
    Console.WriteLine();
    Console.WriteLine("Usage: HeaderReorder <project-path> [options]");
    Console.WriteLine();
    Console.WriteLine("Arguments:");
    Console.WriteLine("  <project-path>    Path to UE5 project root (containing .uproject)");
    Console.WriteLine();
    Console.WriteLine("Options:");
    Console.WriteLine("  --all, -a         Process all files (ignore modification time)");
    Console.WriteLine("  --minutes, -m     Only process files modified within N minutes (default: 10)");
    Console.WriteLine("  --dry-run, -d     Preview changes without modifying files");
    Console.WriteLine("  --verbose, -v     Show detailed output");
    Console.WriteLine("  --help, -h        Show this help");
    Console.WriteLine();
    Console.WriteLine("Examples:");
    Console.WriteLine("  HeaderReorder \"D:\\MyProject\\MyGame\"");
    Console.WriteLine("  HeaderReorder \"D:\\MyProject\\MyGame\" --all --dry-run");
}
