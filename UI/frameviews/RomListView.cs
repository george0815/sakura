
using System.Data;
using Terminal.Gui;

namespace sakura.frameviews;

/// <summary>
/// Displays a list of active torrents with progress, peers, rates, and controls
/// for starting, stopping, seeding, and removing torrents.
/// 
/// This view is backed by a TableView and synchronizes with TorrentOperations events.
/// </summary>
public class RomListView : FrameView
{
    /// <summary>
    /// List of roms backing this view.
    /// </summary>
    private readonly List<Rom> _roms;

    /// <summary>
    /// Terminal.Gui table showing rom info.
    /// </summary>
    private readonly TableView _table;

    /// <summary>
    /// DataTable holding the rom rows displayed in _table.
    /// </summary>
    private readonly DataTable _tableData;

    /// <summary>
    /// Initialize the rom list view with the provided roms.
    /// </summary>
    /// <param name="roms">List of rom objects</param>
    public RomListView(List<Rom> roms)
        : base(Resources.Roms)
    {
        _roms = roms;

        X = 20;
        Y = SettingsData.HeaderHeight;
        Width = Dim.Fill();
        Height = Dim.Fill();

        // --- Table Schema ---
        _tableData = new DataTable();

        // Define visible columns in a consistent order
        _tableData.Columns.Add(Resources.Name, typeof(string));

        // --- TableView Setup ---
        _table = new TableView()
        {
            X = 0,
            Y = 0,
            Width = Dim.Fill(),
            Height = Dim.Fill(),
            Table = _tableData
        };

        Add(_table);

    }



}

