//TODO: IMPLEMENT SAVES VIEW
using sakura;
using Terminal.Gui;

/// <summary>
/// View for options for manging SRAM .sav files and save states.
///
/// Logic is similar to the settings/controls view, but is placed here to separate the responsibilites.
/// </summary>
internal class SavesView : FrameView
{

    public SavesView()
        : base(Resources.Log)
    {
        // Position the frame consistently with other views.
        X = 20;
        Y = SettingsData.HeaderHeight;
        Width = Dim.Fill();
        Height = Dim.Fill();


    }

}
