//TODO: IMPLEMENT CONTROLS VIEW
using sakura;
using Terminal.Gui;

/// <summary>
/// View for options for editing the controls.
///
/// Logic is similar to the settings view, but is placed here to separate the responsibilites.
/// </summary>
internal class ControlsView : FrameView
{

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
