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
            var startButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            startButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.START = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                startButton.Text = ((Key)tmp).ToString();

                // Force redraw
                startButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(startButton);
            y += 2;

            // SELECT
            scroll.Add(new Label("SELECT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.SELECT;
            var selectButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            selectButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.SELECT = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                selectButton.Text = ((Key)tmp).ToString();

                // Force redraw
                selectButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(selectButton);
            y += 2;

            // A
            scroll.Add(new Label("A: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.A;
            var aButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            aButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.A = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                aButton.Text = ((Key)tmp).ToString();

                // Force redraw
                aButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(aButton);
            y += 2;

            // B
            scroll.Add(new Label("B: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.B;
            var bButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            bButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.B = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                bButton.Text = ((Key)tmp).ToString();

                // Force redraw
                bButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(bButton);
            y += 2;

            // UP
            scroll.Add(new Label("UP: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.UP;
            var upButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            upButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.UP = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                upButton.Text = ((Key)tmp).ToString();

                // Force redraw
                upButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(upButton);
            y += 2;

            // DOWN
            scroll.Add(new Label("DOWN: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.DOWN;
            var downButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            downButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.DOWN = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                downButton.Text = ((Key)tmp).ToString();

                // Force redraw
                downButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(downButton);
            y += 2;

            // LEFT
            scroll.Add(new Label("LEFT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.LEFT;
            var leftButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            leftButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.LEFT = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                leftButton.Text = ((Key)tmp).ToString();

                // Force redraw
                leftButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(leftButton);
            y += 2;


            // RIGHT
            scroll.Add(new Label("RIGHT: ") { X = 1, Y = y });
            myKey = Settings.Current.Controls.RIGHT;
            var rightButton = new Button()
            {
                X = 30,
                Y = y,
                Width = 3,
                Height = 1,
                Text = myKey.ToString()
            };

            rightButton.Clicked += () =>
            {
                // Pick color from grid dialog
                int tmp = DialogHelpers.PickKey();
                Settings.Current.Controls.RIGHT = (Key)tmp;

                // Update button label to reflect selection
                char tmpChr = (char)tmp;
                string tmpString = tmpChr.ToString();
                rightButton.Text = ((Key)tmp).ToString();

                // Force redraw
                rightButton.SetNeedsDisplay();
                scroll.SetNeedsDisplay();
            };

            scroll.Add(rightButton);
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
                      Key[] tmpKeys = [
                          Settings.Current.Controls.START,
                          Settings.Current.Controls.SELECT,
                          Settings.Current.Controls.A,
                          Settings.Current.Controls.B,
                          Settings.Current.Controls.UP,
                          Settings.Current.Controls.DOWN,
                          Settings.Current.Controls.LEFT,
                          Settings.Current.Controls.RIGHT,
                      ];

                      if (tmpKeys.Distinct().Count() != tmpKeys.Count())
                      {
                          MessageBox.ErrorQuery(Resources.Error, "Cannot have the same key for multiple buttons.", Resources.OK);
                          return;
                      }

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
