using sakura.frameviews;
using sakura.helpers;
using Terminal.Gui;

namespace sakura
{
    /// <summary>
    /// Main UI window for Lain application using Terminal.Gui.
    /// Contains header, sidebar menu, and various content panels.
    /// </summary>
    public class SakuraUI : Window
    {
        // ------------------------------
        // Content views
        // ------------------------------
        readonly FrameView romListView;
        readonly FrameView savesView;
        readonly FrameView settingsView;
        readonly FrameView controlsView;
        internal FrameView logView;

        // Header components
        readonly private FrameView header;
        readonly private Label romCount;

        public SakuraUI()
        {
            #region HEADER

            // Header container
            header = new FrameView()
            {
                X = 0,
                Y = 0,
                Width = Dim.Fill(),
                Height = Settings.Current.DisableASCII ? 5 : SettingsData.HeaderHeight,
                Border = new Border() { BorderStyle = BorderStyle.None }
            };

            // Scrollable area inside header
            var headerScroll = new ScrollView()
            {
                X = 0,
                Y = 0,
                Width = Dim.Fill(),
                Height = Dim.Fill(),
                ContentSize = new Size(120, SettingsData.HeaderHeight),
                ShowHorizontalScrollIndicator = false,
                ShowVerticalScrollIndicator = false,
                Border = new Border() { BorderStyle = BorderStyle.None }
            };

            header.Add(headerScroll);

            // ASCII logo frame
            var logoFrame = new FrameView()
            {
                X = 0,
                Y = 0,
                Width = Settings.Current.DisableASCII ? 0 : SettingsData.LogoWidth,
                Height = SettingsData.HeaderHeight,
                Border = new Border() { BorderStyle = BorderStyle.Single }
            };

            var logo = new Label()
            {
                X = 0,
                Y = 0,
                Width = Dim.Fill(),
                Height = Dim.Fill(),
                Text = Settings.Current.Icons[0],
                ColorScheme = new ColorScheme()
                {
                    Normal = Application.Driver.MakeAttribute(Settings.Current.LogoColor, Settings.Current.BackgroundColor),
                    Focus = Application.Driver.MakeAttribute(Settings.Current.LogoColor, Settings.Current.BackgroundColor)
                }
            };

            // Add logo only if ASCII art is enabled
            if (!Settings.Current.DisableASCII)
            {
                logoFrame.Add(logo);
                header.Add(logoFrame);
            }

            #region EXTRA HEADER INFO

            // Display current date
            var date = new Label()
            {
                X = (Settings.Current.DisableASCII ? 0 : SettingsData.LogoWidth) + 2,
                Y = 1,
                Text = DateTime.Now.ToString("yyyy-MM-dd")
            };

            // Display number of roms
            romCount = new Label()
            {
                X = (Settings.Current.DisableASCII ? 30 : SettingsData.LogoWidth) + 2,
                Y = (Settings.Current.DisableASCII ? 1 : 3),
                Text = $"{"Roms: "}{"1"}"
            };



            // Add info labels to header scroll
            headerScroll.Add(date, romCount);

            // Add header to main window
            Add(header);

            #endregion


            #endregion

            #region SIDEBAR AND MENU

            // Sidebar container
            var sidebar = new FrameView()
            {
                X = 0,
                Y = SettingsData.HeaderHeight,
                Width = 20,
                Height = Dim.Fill(),
                Border = new Border() { BorderStyle = BorderStyle.Rounded }
            };

            // Sidebar menu
            var menu = new ListView(new string[]
            {
                "Roms",
                "Saves",
                "Settings",
                "Controls",
                "Log",
            })
            {
                X = 1,
                Y = 0,
                Width = Dim.Fill(),
                Height = Dim.Fill() - 3
            };

            // Switch content panel when menu selection changes
            menu.SelectedItemChanged += (args) =>
            {
                SwitchPanel(args.Item, ref logo);
            };

            sidebar.Add(menu);

            // Exit button
            var exitButton = new Button("Exit")
            {
                X = 1,
                Y = Pos.Bottom(menu),
                Width = 16
            };

            exitButton.Clicked += () => ShowExitDialog();

            sidebar.Add(exitButton);

            // Add sidebar to window
            Add(sidebar);

            #endregion


            #region CONTENT VIEWS

            romListView = new RomListView(new List<string>() { "test" });
            savesView = new SavesView();
            settingsView = new SettingsView();
            controlsView = new ControlsView();
            logView = new LogView(Log.LogList);

            // Show default panel
            Add(romListView);

            #endregion
        }

        #region HELPER METHODS


        /// <summary>
        /// Shows exit confirmation dialog and handles application stop.
        /// </summary>
        private static void ShowExitDialog()
        {
            var dialog = new Dialog("Exit", 50, 10) { Height = 3 };
            var ok = new Button("Ok");
            var cancel = new Button("Cancel");

            bool exitRequested = false;

            ok.Clicked += () =>
            {
                exitRequested = true;
                Application.RequestStop(dialog);
            };

            cancel.Clicked += () =>
            {
                Application.RequestStop(dialog);
            };

            dialog.AddButton(ok);
            dialog.AddButton(cancel);

            // Run dialog modally
            Application.Run(dialog);

            // If confirmed, close main UI
            if (exitRequested)
            {
                Application.Top.RemoveAll();
                Application.RequestStop();

            }
        }

        /// <summary>
        /// Switches visible content panel based on menu index.
        /// </summary>
        private void SwitchPanel(int index, ref Label logo)
        {
            // Remove all panels first
            Remove(romListView);
            Remove(savesView);
            Remove(settingsView);
            Remove(controlsView);
            Remove(logView);

            // Update ASCII logo
            logo.Text = ASCII.icons[index];

            switch (index)
            {
                case 0: Add(romListView); break;
                case 1: Add(savesView); break;
                case 2: Add(settingsView); break;
                case 3: Add(controlsView); break;
                case 6: Add(logView); break;
            }

            SetNeedsDisplay();
        }

        /// <summary>
        /// Controls cursor visibility based on settings.
        /// </summary>
        public override void PositionCursor()
        {
            if (Settings.Current.HidetextCursor)
            {
                Application.Driver.SetCursorVisibility(CursorVisibility.Invisible);
                return;
            }

            base.PositionCursor();
        }

        #endregion
    }
}
