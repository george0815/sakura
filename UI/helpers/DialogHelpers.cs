
using Terminal.Gui;
using System;
using System.IO;
using sakura;
using sakura.frameviews;

namespace sakura.helpers
{

    /// <summary>
    /// Provides helper methods to display dialogs for selecting categories, sources,
    /// colors, files, folders, and sorting criteria using Terminal.Gui.
    /// </summary>
    public static class DialogHelpers
    {
        /// <summary>
        /// Displays a dialog allowing multiple selection of categories.
        /// </summary>
        /// <param name="categories">An array of category names to display.</param>
        /// <returns>An array of selected category names.</returns>
        public static string[] PickCategories(string[] categories)
        {
            // Create the dialog window with title and size
            var dlg = new Dialog(Resources.Choosecategories, 60, 12);
            var selected = new HashSet<string>(); // Stores selected categories

            int y = 0;
            foreach (var cat in categories)
            {
                // Create a button for each category
                var btn = new Button($" [ ] {cat} ")
                {
                    X = 1, // Horizontal position
                    Y = y++ // Vertical position increments for each button
                };

                // Toggle selection and update display when button is clicked
                btn.Clicked += () =>
                {
                    if (selected.Remove(cat))
                    {
                        btn.Text = $" [ ] {cat} ";
                    }
                    else
                    {
                        selected.Add(cat);
                        btn.Text = $" [x] {cat} ";
                    }
                };

                dlg.Add(btn); // Add button to dialog
            }

            // OK button stops dialog and returns selection
            var okBtn = new Button(Resources.OK) { IsDefault = true };
            okBtn.Clicked += () => Application.RequestStop();

            // Cancel button clears selection and closes dialog
            var cancelBtn = new Button(Resources.Cancel);
            cancelBtn.Clicked += () =>
            {
                selected.Clear();
                Application.RequestStop();
            };

            dlg.AddButton(okBtn);     // Add OK button to dialog
            dlg.AddButton(cancelBtn); // Add Cancel button to dialog

            Application.Run(dlg);     // Run the dialog
            return [.. selected]; // Return selected categories
        }

        /// <summary>
        /// Displays a dialog allowing multiple selection of sources.
        /// </summary>
        /// <param name="sources">An array of source names to display.</param>
        /// <returns>An array of selected source names.</returns>
        public static string[] PickSources(string[] sources)
        {
            var dlg = new Dialog(Resources.Choosesources, 60, 12);
            var selected = new HashSet<string>();

            int y = 0;
            foreach (var src in sources)
            {
                // Create a button for each source
                var btn = new Button($" [ ] {src} ")
                {
                    X = 1,
                    Y = y++
                };

                // Toggle selection when button is clicked
                btn.Clicked += () =>
                {
                    if (selected.Remove(src))
                    {
                        btn.Text = $" [ ] {src} ";
                    }
                    else
                    {
                        selected.Add(src);
                        btn.Text = $" [x] {src} ";
                    }
                };

                dlg.Add(btn); // Add button to dialog
            }

            var okBtn = new Button(Resources.OK) { IsDefault = true };
            okBtn.Clicked += () => Application.RequestStop(); // Stop dialog on OK

            var cancelBtn = new Button(Resources.Cancel);
            cancelBtn.Clicked += () =>
            {
                selected.Clear(); // Clear selection on cancel
                Application.RequestStop();
            };

            dlg.AddButton(okBtn);
            dlg.AddButton(cancelBtn);

            Application.Run(dlg);
            return [.. selected];
        }

        /// <summary>
        /// Displays a dialog allowing single selection of a sort criterion.
        /// </summary>
        /// <param name="criteria">An array of sorting criteria.</param>
        /// <returns>The selected criterion, or empty string if canceled.</returns>
        public static string PickSortCriteria(string[] criteria)
        {
            var dlg = new Dialog("Sort by: ", 40, 10);
            string result = criteria.FirstOrDefault() ?? ""; // Default selection

            int y = 0;
            Button? active = null; // Track the currently selected button

            foreach (var c in criteria)
            {
                // Create a radio-style button for each criterion
                var btn = new Button($" ( ) {c} ")
                {
                    X = 1,
                    Y = y++
                };

                // Click handler updates active selection
                btn.Clicked += () =>
                {
                    active?.Text = active.Text.Replace("(x)", "( )"); // Deselect previous

                    btn.Text = $" (x) {c} "; // Select this one
                    active = btn;
                    result = c;
                };

                dlg.Add(btn);

                // Initialize first button as active if none yet
                if (active == null)
                {
                    active = btn;
                    btn.Text = $" (x) {c} ";
                    result = c;
                }
            }

            var okBtn = new Button(Resources.OK) { IsDefault = true };
            okBtn.Clicked += () => Application.RequestStop();

            var cancelBtn = new Button(Resources.Cancel);
            cancelBtn.Clicked += () =>
            {
                result = ""; // Clear result if canceled
                Application.RequestStop();
            };

            dlg.AddButton(okBtn);
            dlg.AddButton(cancelBtn);

            Application.Run(dlg);
            return result;
        }

        /// <summary>
        /// Displays a dialog to pick a color from a grid.
        /// </summary>
        /// <returns>The name of the selected color.</returns>
        public static string PickColorGrid()
        {
            var colors = SettingsView.colors;
            var dlg = new Dialog(Resources.ChooseColor, 50, 8)
            {
                // Set the color scheme for the dialog window
                ColorScheme = new ColorScheme()
                {
                    Normal = Application.Driver.MakeAttribute(Settings.Current.TextColor, Color.Black),
                    Focus = Application.Driver.MakeAttribute(Settings.Current.FocusTextColor, Color.Black),
                    HotNormal = Application.Driver.MakeAttribute(Settings.Current.HotTextColor, Color.Black),
                    HotFocus = Application.Driver.MakeAttribute(Settings.Current.FocusTextColor, Color.Black),
                }
            };

            string result = Resources.Black; // Default color

            int x = 0, y = 0;
            foreach (var c in colors)
            {
                // Adjust text color for visibility on black background
                var textColor = c.Value == Color.Black ? Color.White : c.Value;

                var box = new Button($" {c.Key} ")
                {
                    X = x * 15, // Grid column
                    Y = y,      // Grid row
                    ColorScheme = new ColorScheme()
                    {
                        Normal = new Terminal.Gui.Attribute(textColor, Color.Black),
                        Focus = Application.Driver.MakeAttribute(textColor, Color.Black),
                        HotNormal = Application.Driver.MakeAttribute(textColor, Color.Black),
                        HotFocus = Application.Driver.MakeAttribute(textColor, Color.Black),
                    }
                };

                // Set color selection on click
                box.Clicked += () =>
                {
                    result = c.Key;
                    Application.RequestStop();
                };

                dlg.Add(box);

                x++; // Move to next column
                if (x == 3) { x = 0; y++; } // Wrap to next row after 3 columns
            }

            Application.Run(dlg);
            return result;
        }

        /// <summary>
        /// Shows a save file dialog with validation for allowed extensions and directories.
        /// </summary>
        public static string? ShowSaveFileDialog(string title, string message, string[] allowedExtensions, string defaultFileName = "")
        {
            var dialog = new SaveDialog(title, message)
            {
                CanCreateDirectories = true, // Allow directory creation
                AllowedFileTypes = allowedExtensions,
                FilePath = defaultFileName
            };

            Application.Run(dialog);

            // Check if user canceled
            if (dialog.Canceled || string.IsNullOrWhiteSpace(dialog.FilePath?.ToString()))
                return null;

            string? path = dialog.FilePath!.ToString();

            try
            {
                // Ensure directory exists or create it
                var dir = Path.GetDirectoryName(path);
                if (string.IsNullOrWhiteSpace(dir))
                    throw new Exception(Resources.Invalidfile_folderpath);

                if (!Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                return path;
            }
            catch (Exception ex)
            {
                // Show error message
                MessageBox.ErrorQuery(40, 10, Resources.Error, ex.Message, Resources.OK);
                return null;
            }
        }

        /// <summary>
        /// Shows an open file dialog with optional validation for existing files.
        /// </summary>
        public static string? ShowFileDialog(string title, string message, string[] allowedExtensions, bool create)
        {
            var dialog = new OpenDialog(title, message)
            {
                CanChooseFiles = true,
                CanChooseDirectories = false,
                AllowedFileTypes = allowedExtensions,
                AllowsMultipleSelection = false
            };

            Application.Run(dialog);

            if (!dialog.Canceled && dialog.FilePaths.Count > 0)
            {
                var path = dialog.FilePath.ToString();

                if (!create)
                {
                    if (File.Exists(path)) { return path; } // Valid file exists
                    else { MessageBox.ErrorQuery(Resources.InvalidFile, Resources.Selectedfiledoesnotexist, Resources.OK); }
                }
                else
                {
                    return path; // Return path even if file doesn't exist yet
                }
            }

            return null;
        }

        /// <summary>
        /// Truncates a string to a maximum length.
        /// </summary>
        internal static string Truncate(this string value, int maxLength)
        {
            if (string.IsNullOrEmpty(value)) return value; // Nothing to truncate
            return value.Length <= maxLength ? value : value[..maxLength]; // Truncate if necessary
        }

        /// <summary>
        /// Shows a folder selection dialog with validation.
        /// </summary>
        public static string? ShowFolderDialog(string title, string message)
        {
            var dialog = new OpenDialog(title, message)
            {
                CanChooseFiles = false,
                CanChooseDirectories = true,
                AllowsMultipleSelection = false
            };

            Application.Run(dialog);

            if (!dialog.Canceled && dialog.FilePaths.Count > 0)
            {
                var path = dialog.FilePath.ToString();

                if (Directory.Exists(path))
                    return path; // Valid folder
                else
                    MessageBox.ErrorQuery(Resources.InvalidFolder, Resources.Selectedfolderdoesnotexist, Resources.OK);
            }

            return null; // Canceled or invalid
        }
    }
}
