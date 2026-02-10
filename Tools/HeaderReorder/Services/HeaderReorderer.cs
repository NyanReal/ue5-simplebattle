using System.Text.RegularExpressions;

namespace HeaderReorder.Services;

/// <summary>
/// Result of processing a header file.
/// </summary>
public record ProcessResult(bool WasModified, string? MovedInclude);

/// <summary>
/// Reorders #include statements to ensure .generated.h is always last.
/// </summary>
public partial class HeaderReorderer
{
    // Match #include "Something.generated.h"
    [GeneratedRegex(@"^\s*#\s*include\s*""([^""]+\.generated\.h)""\s*$", RegexOptions.IgnoreCase)]
    private static partial Regex GeneratedIncludeRegex();

    // Match any #include statement
    [GeneratedRegex(@"^\s*#\s*include\s*[<""]([^>""]+)[>""]")]
    private static partial Regex AnyIncludeRegex();

    /// <summary>
    /// Process a single header file.
    /// </summary>
    public ProcessResult ProcessFile(string filePath, bool dryRun)
    {
        var lines = File.ReadAllLines(filePath);
        var result = ReorderIncludes(lines);

        if (!result.WasModified)
        {
            return new ProcessResult(false, null);
        }

        if (!dryRun)
        {
            File.WriteAllLines(filePath, result.NewLines);
        }

        return new ProcessResult(true, result.MovedInclude);
    }

    private (bool WasModified, string[] NewLines, string? MovedInclude) ReorderIncludes(string[] lines)
    {
        // Find the include block and the .generated.h include
        int generatedIncludeLine = -1;
        int lastIncludeLine = -1;
        string? generatedInclude = null;

        for (int i = 0; i < lines.Length; i++)
        {
            var line = lines[i];

            // Check if this is any include
            if (AnyIncludeRegex().IsMatch(line))
            {
                lastIncludeLine = i;

                // Check if this is a .generated.h include
                var match = GeneratedIncludeRegex().Match(line);
                if (match.Success)
                {
                    generatedIncludeLine = i;
                    generatedInclude = match.Groups[1].Value;
                }
            }
        }

        // If no .generated.h found, or it's already the last include, nothing to do
        if (generatedIncludeLine < 0 || generatedIncludeLine == lastIncludeLine)
        {
            return (false, lines, null);
        }

        // Need to move the .generated.h include to after the last include
        var newLines = new List<string>();
        string generatedLine = lines[generatedIncludeLine];

        for (int i = 0; i < lines.Length; i++)
        {
            // Skip the original .generated.h line
            if (i == generatedIncludeLine)
            {
                continue;
            }

            newLines.Add(lines[i]);

            // Insert .generated.h after the last include
            if (i == lastIncludeLine)
            {
                newLines.Add(generatedLine);
            }
        }

        return (true, newLines.ToArray(), generatedInclude);
    }
}
