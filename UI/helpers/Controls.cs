using Terminal.Gui;

namespace sakura.helpers
{
    /// <summary>
    /// Represents a set of configurable hotkeys for rom operations.
    /// This struct holds the key mappings for starting/stopping roms and opening path folders.
    /// </summary>
    public struct RomHotkeys
    {
        /// <summary>
        /// Initializes a new instance of the <see cref="RomHotkeys"/> struct.
        /// The default keys are assigned automatically via property initializers.
        /// </summary>
        public RomHotkeys() { }

        /// <summary>
        /// The key used to start/resume a rom download.
        /// Default is F3.
        /// </summary>
        public Key StartRom { get; set; } = Key.F3;

        /// <summary>
        /// The key used to stop a rom.
        /// Default is F4.
        /// </summary>
        public Key StopRom { get; set; } = Key.F4;

        /// <summary>
        /// The key used to open the rom's path.
        /// Default is F5.
        /// </summary>
        public Key OpenRomPath { get; set; } = Key.F5;

        /// <summary>
        /// The key used to open a rom's SRAM .sav folder.
        /// Default is F6.
        /// </summary>
        public Key OpenSramPath { get; set; } = Key.F6;


    }
}
