//TODO: DOC STRINGS, COMMENTS, AND PARSING ROM DATA

using sakura.helpers;

namespace sakura;

//Class that represents NES rom
public class Rom
{

    public string? Name { get; set; }

    public string? RomPath { get; set; }

    public string? SramPath { get; set; }

    public bool NTSC { get; set; }


    //For all rom directories, get all roms in each directoy and return list of all roms
    internal static List<Rom> GetAllRoms()
    {

        List<Rom> roms = new List<Rom>();
        foreach (string path in Settings.Current.AllRomPaths)
        {
            if (Directory.Exists(path))
            {
                roms.AddRange(GetRoms(path));
            }
        }

        return roms;

    }

    private static List<Rom> GetRoms(string path)
    {


        List<Rom> tempRoms = new List<Rom>();
        DirectoryInfo DirInfo = new DirectoryInfo(path);

        foreach (FileInfo file in DirInfo.EnumerateFiles())
        {
            string ext = Path.GetExtension(file.Name);

            if (ext.ToLower() == ".nes")
            {

                Log.Write(file.Name);

                //create temp rom
                string name = Path.GetFileNameWithoutExtension(file.Name);
                Rom tempRom = new Rom()
                {
                    Name = name,
                    RomPath = Path.Combine(file.DirectoryName!, file.Name),
                    SramPath = Path.Combine(Settings.Current.SramPath!, name),
                    NTSC = true,
                };
                tempRoms.Add(tempRom);

            }
        }

        //For every .nes file in directory, call the parser
        foreach (FileInfo file in DirInfo.EnumerateFiles().Where(
              f => Path.GetExtension(f.Name!)
              .Equals("nes", StringComparison.OrdinalIgnoreCase)))
        {
            //call parser TODO: IMPLEMENT PARSER 


        }


        return tempRoms;
    }

}


