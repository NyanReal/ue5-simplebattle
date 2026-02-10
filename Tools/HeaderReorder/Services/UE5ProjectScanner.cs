namespace HeaderReorder.Services;

/// <summary>
/// Scans UE5 project directories for header files.
/// </summary>
public class UE5ProjectScanner
{
    private readonly string _projectRoot;
    
    private static readonly string[] ExcludedFolders = 
    {
        "Intermediate",
        "Binaries", 
        "Saved",
        "DerivedDataCache",
        ".git",
        ".vs"
    };

    public UE5ProjectScanner(string projectRoot)
    {
        _projectRoot = Path.GetFullPath(projectRoot);
    }

    /// <summary>
    /// Validates if the given path is a UE5 project.
    /// </summary>
    public static bool IsValidUE5Project(string path)
    {
        if (!Directory.Exists(path))
            return false;

        // Check for .uproject file
        return Directory.GetFiles(path, "*.uproject").Length > 0;
    }

    /// <summary>
    /// Scans for .h files in Source and Plugins directories.
    /// </summary>
    /// <param name="modifiedAfter">Only include files modified after this time.</param>
    public List<string> ScanHeaderFiles(DateTime modifiedAfter)
    {
        var headerFiles = new List<string>();

        // Scan main Source directory
        var sourceDir = Path.Combine(_projectRoot, "Source");
        if (Directory.Exists(sourceDir))
        {
            headerFiles.AddRange(ScanDirectory(sourceDir, modifiedAfter));
        }

        // Scan Plugins directories
        var pluginsDir = Path.Combine(_projectRoot, "Plugins");
        if (Directory.Exists(pluginsDir))
        {
            foreach (var pluginDir in Directory.GetDirectories(pluginsDir))
            {
                var pluginSourceDir = Path.Combine(pluginDir, "Source");
                if (Directory.Exists(pluginSourceDir))
                {
                    headerFiles.AddRange(ScanDirectory(pluginSourceDir, modifiedAfter));
                }
            }
        }

        return headerFiles;
    }

    private IEnumerable<string> ScanDirectory(string directory, DateTime modifiedAfter)
    {
        var results = new List<string>();

        try
        {
            foreach (var file in Directory.EnumerateFiles(directory, "*.h", SearchOption.AllDirectories))
            {
                // Check if in excluded folder
                if (IsInExcludedFolder(file))
                    continue;

                // Check modification time
                var lastWrite = File.GetLastWriteTime(file);
                if (lastWrite >= modifiedAfter)
                {
                    results.Add(file);
                }
            }
        }
        catch (UnauthorizedAccessException)
        {
            // Skip directories we can't access
        }

        return results;
    }

    private bool IsInExcludedFolder(string filePath)
    {
        var relativePath = Path.GetRelativePath(_projectRoot, filePath);
        var parts = relativePath.Split(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

        return parts.Any(p => ExcludedFolders.Contains(p, StringComparer.OrdinalIgnoreCase));
    }
}
