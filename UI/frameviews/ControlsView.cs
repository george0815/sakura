//TODO: IMPLEMENT CONTROLS VIEW AND RESOURCE STRING
using Terminal.Gui;
using sakura.helpers;

namespace sakura.frameviews
{
    /// <summary>
    /// ControlsView contains buttons that set the controls for the emulator.
    /// </summary>
    internal class ControlsView : FrameView
    {

        /// <summary>
        /// Initializes the controls UI and wires all controls
        /// directly to <see cref="Settings.Current"/>.
        ///
        /// Layout is built procedurally to keep ordering explicit
        /// and easy to modify without hidden layout logic.
        /// </summary>
        public ControlsView()
            : base("Controls")
        {
            X = 20;
            Y = SettingsData.HeaderHeight;
            Width = Dim.Fill();
            Height = Dim.Fill();

            // ScrollView allows all settings to remain accessible
            // even on small or resized terminal windows.
            var scroll = new ScrollView()
            {
                X = 0,
                Y = 0,
                Width = Dim.Fill(),
                Height = Dim.Fill(),
                ShowVerticalScrollIndicator = true,
                ShowHorizontalScrollIndicator = false,
            };

            Add(scroll);

            // Tracks vertical layout position inside the scroll view.
            // This avoids implicit layout rules and keeps spacing obvious.
            int y = 1;


            #region CONTROLS SETTINGS

            // START 
            scroll.Add(new Label("START: ") { X = 1, Y = y });
            var myKey = Settings.Current.Controls.START;
            var bgColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            bgColorCombo.Clicked += () =>
            {
                // Pick color from grid dialog
                string tmp = DialogHelpers.PickKey();

                // Update button label to reflect selection
                bgColorCombo.Text = tmp;

                // Force redraw
                bgColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(bgColorCombo);
            y += 2;

            // SELECT
            scroll.Add(new Label("SELECT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.SELECT;
            var textColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            textColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                textColorCombo.Text = tmp;
                textColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(textColorCombo);
            y += 2;

            // A
            scroll.Add(new Label("A: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.A;
            var backgroundFocusColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            backgroundFocusColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                backgroundFocusColorCombo.Text = tmp;
                backgroundFocusColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(backgroundFocusColorCombo);
            y += 2;

            // B
            scroll.Add(new Label("B: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.B;
            var textFocusColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            textFocusColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                textFocusColorCombo.Text = tmp;
                textFocusColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(textFocusColorCombo);
            y += 2;

            // UP
            scroll.Add(new Label("UP: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.UP;
            var hotTextColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            hotTextColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                hotTextColorCombo.Text = tmp;
                hotTextColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(hotTextColorCombo);
            y += 2;

            // DOWN
            scroll.Add(new Label("DOWN: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.DOWN;
            var logoColorCombo = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            logoColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                logoColorCombo.Text = tmp;
                logoColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(logoColorCombo);
            y += 2;

            // LEFT
            scroll.Add(new Label("LEFT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.LEFT;
            var left = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            logoColorCombo.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                logoColorCombo.Text = tmp;
                logoColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(left);
            y += 2;


            // RIGHT
            scroll.Add(new Label("RIGHT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.RIGHT;
            var right = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            right.Clicked += () =>
            {
                string tmp = DialogHelpers.PickKey();
                logoColorCombo.Text = tmp;
                logoColorCombo.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(right);
            y += 2;


            #endregion

            // Persist settings to disk
            var saveBtn = new Button(Resources.Save) { X = 1, Y = y };
            scroll.Add(saveBtn);

            // Required so scroll bars calculate correctly
            scroll.ContentSize =
                new Terminal.Gui.Size(Application.Top.Frame.Width - 2, y + 2);

            saveBtn.Clicked += () =>
              {
                  try
                  {
                      // --- controls validation ---


                      // --- apply settings ---
                      Settings.Save();

                      MessageBox.Query(Resources.Settings, Resources.Settingssavedsuccessfully, Resources.OK);
                      Log.Write(Resources.Settingssaved);
                  }
                  catch (Exception ex)
                  {
                      MessageBox.ErrorQuery(Resources.FatalError, ex.Message, Resources.OK);
                  }
              };


        }
    }
}
