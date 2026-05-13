


using sakura;
using sakura.helpers;
using Terminal.Gui;

/// <summary>
/// Read-only view used to display application log output.
///
/// This view listens for log updates and keeps the ListView in sync
/// while attempting to preserve user context (scroll position and
/// selected row) across refreshes.
/// </summary>
internal class ControlsView : FrameView
{
    /// <summary>
    /// ListView displaying the current log entries.
    ///
    /// Exposed to allow external configuration if needed, but is
    /// fully initialized by the view itself.
    /// </summary>

    public ControlsView()
        : base(Resources.Log)
    {
        // Position the frame consistently with other views.
        X = 20;
        Y = SettingsData.HeaderHeight;
        Width = Dim.Fill();
        Height = Dim.Fill();


    }

}
