using System;
using System.IO;
using System.Collections;
using Gtk;
using Gdk;
using Gnome;
using Glade;

public class Application {
    [Glade.Widget]
    private Gtk.Window MainWindow;
    [Glade.Widget]
    private Gtk.DrawingArea DrawingArea;
    [Glade.Widget]
    private Gtk.CheckButton SolidCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton UniSolidCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton IceCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton WaterCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton SlopeCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton DontUseCheckButton;
    [Glade.Widget]
    private Gtk.CheckButton HiddenCheckButton;
    [Glade.Widget]
    private Gtk.Entry DataEntry;
    [Glade.Widget]
    private Gtk.Entry AnimFpsEntry;
    [Glade.Widget]
    private Gtk.Entry IDEntry;
    [Glade.Widget]
    private Gnome.AppBar AppBar;
    [Glade.Widget]
    private Gtk.VBox MainLayout;
    [Glade.Widget]
    private Gtk.TreeView TileList;
    [Glade.Widget]
    private Gtk.Combo TileGroupComboBox;
    [Glade.Widget]
    private Gtk.MenuItem AddTileGroupMenu;

    private string tilesetdir;
    private string tilesetfile;
    private TileSet tileset;
    private TileGroup selectedgroup;

    private Tile[] Tiles;
    private bool[] SelectionArray;
    private ArrayList Selection = new ArrayList();
    private int TilesX;
    private int TilesY;
    private bool toggling;
    private bool selecting;

    private string currentimage;
    private Gdk.Pixbuf pixbuf;

    public static int Main(string[] args) {
        Program kit = new Program("tiler", "0.0.1", Modules.UI, args);

        Application app = new Application();

        /* that's no proper commandlineparsing, but who'll notice... */
        if(args.Length == 1)
            app.LoadTileSet(args[0]);

        kit.Run();
        return 0;
    }

    public Application() {
        Glade.XML gxml = new Glade.XML(null, "tiler.glade", null, null);
        gxml.Autoconnect(this);

        if(MainWindow == null || DrawingArea == null || AppBar == null)
            throw new Exception("soem widgets not found");

        DrawingArea.AddEvents((int) Gdk.EventMask.ButtonPressMask);
        DrawingArea.AddEvents((int) Gdk.EventMask.ButtonReleaseMask);
        DrawingArea.AddEvents((int) Gdk.EventMask.ButtonMotionMask);

        // libglade missed interactivity property :-/
        MainLayout.Remove(AppBar);
        AppBar = new AppBar(true, true, PreferencesType.Always);
        AppBar.UserResponse += new EventHandler(OnAppBarUserResponse);
        MainLayout.PackStart(AppBar, false, false, 0);
        AppBar.Show();

        TileGroupComboBox.Entry.Activated
            += new EventHandler (OnTileGroupComboBoxEntryActivated);

        MainWindow.Show();
    }

    private void OnOpen(object o, EventArgs e) {
        FileSelection selection = new FileSelection("Select TileSet");
        selection.OkButton.Clicked += new EventHandler(OnSelectTileSetOk);
        selection.CancelButton.Clicked += new EventHandler(OnSelectImageCancel);
        selection.Show();
    }

    private void OnSelectTileSetOk(object o, EventArgs e) {
        FileSelection selection = ((FileSelection.FSButton) o).FileSelection;
        string file = selection.Filename;
        selection.Destroy();

        LoadTileSet(file);
    }

    private void LoadTileSet(string file) {
        try {
            tileset = new TileSet();
            tileset.Parse(file);
            tilesetfile = file;
            tilesetdir = new FileInfo(file).Directory.ToString();
        } catch(Exception exception) {
            ShowException(exception);
        }

        Selection.Clear();
        SelectionChanged();
        FillTileGroupComboBox();
        FillTileList();
    }

    private void OnImportImage(object o, EventArgs e) {
        FileSelection selection = new FileSelection("Select ImageFile");
        selection.OkButton.Clicked += new EventHandler(OnSelectImageOk);
        selection.CancelButton.Clicked += new EventHandler(OnSelectImageCancel);
        selection.Show();
    }

    private void OnSelectImageCancel(object o, EventArgs args) {
        FileSelection selection = ((FileSelection.FSButton) o).FileSelection;
        selection.Destroy();
    }

    private void OnSelectImageOk(object o, EventArgs args) {
        FileSelection selection = ((FileSelection.FSButton) o).FileSelection;
        string file = selection.Filename;
        selection.Destroy();

        ChangeImage(new FileInfo(file).Name);

        int startid = tileset.Tiles.Count;
        for(int y = 0; y < TilesY; ++y) {
            for(int x = 0; x < TilesX; ++x) {
                int i = y*TilesX+x;
                Tile tile = new Tile();
                tile.ID = startid + i;
                ImageRegion region = new ImageRegion();
                region.ImageFile = currentimage;
                region.Region = new System.Drawing.Rectangle(x*32, y*32, 32, 32);
                tile.Images.Add(region);
                if(Tiles[i] != null) {
                    Console.WriteLine(
                            "Warning Tile in this region already existed...");
                }
                Tiles[i] = tile;
                tileset.Tiles.Add(tile);
            }
        }

        FillTileList();
    }

    private void ChangeImage(string file) {
        if(file == "") {
            currentimage = "";
            pixbuf = null;
            return;
        }
        try {
            pixbuf = new Pixbuf(tilesetdir + "/" + file);
            if(pixbuf.Width % 32 != 0 || pixbuf.Height % 32 != 0)
                Console.WriteLine("Warning: Image Width or Height is not a multiple of 32");
        } catch(Exception e) {
            ShowException(e);
            return;
        }
        currentimage = new FileInfo(file).Name;
        TilesX = pixbuf.Width / 32;
        TilesY = pixbuf.Height / 32;
        SelectionArray = new bool[TilesX * TilesY];
        Tiles = new Tile[TilesX * TilesY];

        // search tileset for tiles with matching image
        foreach(Tile tile in tileset.Tiles) {
            if(tile == null)
                continue;
            if(tile.Images.Count == 0)
                continue;
            ImageRegion region = (ImageRegion) tile.Images[0];
            if(region.ImageFile == currentimage) {
                int px = region.Region.X / 32;
                int py = region.Region.Y / 32;
                int i = py*TilesX+px;
                if(i < 0 || i >= Tiles.Length) {
                    Console.WriteLine("Invalid Imageregion at tile " +
                            tile.ID);
                    continue;
                }
                if(Tiles[i] != null) {
                    Console.WriteLine("Multiple tiles for region " +
                            px*32 + " , " + py*32);
                    continue;
                }
                Tiles[i] = tile;
            }
        }

        /*   DrawingArea.Allocation
            = new Gdk.Rectangle(0, 0, pixbuf.Width, pixbuf.Height);*/
        DrawingArea.WidthRequest = pixbuf.Width;
        DrawingArea.HeightRequest = pixbuf.Height;
        DrawingArea.QueueResize();
    }

    private void OnSave(object o, EventArgs e) {
        tileset.Write(tilesetfile);
    }

    private void OnQuit(object o, EventArgs e) {
        Gtk.Application.Quit();
    }

    private void OnAbout(object o, EventArgs e) {
    }

    private void OnRemapTiles(object o, EventArgs e) {
        AppBar.SetPrompt("Start-ID:", true);
    }

    private void OnCreateTileGroup(object o, EventArgs e) {
    }

    private void OnRenameTileGroup(object o, EventArgs e) {
    }

    private void OnAppBarUserResponse(object o, EventArgs e) {
        try {
            if(AppBar.Response == null || AppBar.Response == ""
                    || Tiles == null)
                return;

            // remap tiles
            int id;
            try {
                id = Int32.Parse(AppBar.Response);
            } catch(Exception exception) {
                ShowException(exception);
                return;
            }
            foreach(Tile tile in Selection) {
                if(tile.ID == -1)
                    continue;

                int oldid = tile.ID;
                tile.ID = id++;
                // remap in all tilegroups...
                foreach(TileGroup tilegroup in tileset.TileGroups) {
                    int idx = tilegroup.Tiles.IndexOf(oldid);
                    if(idx >= 0) {
                        tilegroup.Tiles[idx] = tile.ID;
                    }
                }
            }
            FillTileList();
            SelectionChanged();
        } finally {
            AppBar.ClearPrompt();
        }
    }

    private void OnDrawingAreaExpose(object o, ExposeEventArgs e) {
        if(pixbuf == null)
            return;

        Drawable drawable = e.Event.Window;
        Gdk.GC gc = new Gdk.GC(drawable);
        drawable.DrawPixbuf(gc, pixbuf, 0, 0, 0, 0,
                pixbuf.Width, pixbuf.Height, RgbDither.None, 0, 0);

        gc.RgbFgColor = new Color(0xff, 0, 0);
        foreach(Tile tile in Selection) {
            System.Drawing.Rectangle rect
                = ((ImageRegion) tile.Images[0]).Region;
            drawable.DrawRectangle(gc, false, rect.X, rect.Y, rect.Width,
                    rect.Height);
        }

        e.RetVal = false;
    }

    private void OnDrawingAreaButtonPress(object o, ButtonPressEventArgs e) {
        if(SelectionArray == null)
            return;

        selecting = true;

        for(int i = 0; i < SelectionArray.Length; ++i)
            SelectionArray[i] = false;
        select((int) e.Event.X, (int) e.Event.Y);
    }

    private void select(int x, int y) {
        int tile = y/32 * TilesX + x/32;
        if(tile < 0 || tile >= SelectionArray.Length)
            return;

        SelectionArray[tile] = true;
        SelectionArrayChanged();
    }

    private void OnDrawingAreaMotionNotify(object i, MotionNotifyEventArgs e) {
        if(!selecting)
            return;
        select((int) e.Event.X, (int) e.Event.Y);
    }

    private void OnDrawingAreaButtonRelease(object o, ButtonPressEventArgs e) {
        selecting = false;
    }

    private void OnCheckButtonToggled(object sender, EventArgs e) {
        if(toggling)
            return;
        foreach(Tile tile in Selection) {
            if(sender == SolidCheckButton)
                tile.Solid = SolidCheckButton.Active;
            if(sender == UniSolidCheckButton)
                tile.UniSolid = UniSolidCheckButton.Active;
            if(sender == IceCheckButton)
                tile.Ice = IceCheckButton.Active;
            if(sender == WaterCheckButton)
                tile.Water = WaterCheckButton.Active;
            if(sender == SlopeCheckButton)
                tile.Slope = SlopeCheckButton.Active;
            if(sender == HiddenCheckButton)
                tile.Hidden = HiddenCheckButton.Active;
            if(sender == DontUseCheckButton)
                tile.ID = DontUseCheckButton.Active ? -1 : 0;
        }
    }

    private void OnEntryChanged(object sender, EventArgs e) {
        if(toggling)
            return;
        foreach(Tile tile in Selection) {
            try {
                if(sender == IDEntry)
                    tile.ID = Int32.Parse(IDEntry.Text);
                if(sender == DataEntry)
                    tile.Data = Int32.Parse(DataEntry.Text);
                if(sender == AnimFpsEntry)
                    tile.AnimFps = Single.Parse(AnimFpsEntry.Text);
            } catch(Exception exception) {
                // ignore parse errors for now...
            }
        }
    }

    private void SelectionArrayChanged() {
        Selection.Clear();
        for(int i = 0; i < SelectionArray.Length; ++i) {
            if(!SelectionArray[i])
                continue;

            if(Tiles[i] == null) {
                Console.WriteLine("Tile doesn't exist yet");
                // TODO ask user to create new tile...
                continue;
            }
            Selection.Add(Tiles[i]);
        }

        SelectionChanged();
    }

    private void SelectionChanged() {
        bool first = true;
        toggling = true;
        string nextimage = "";
        foreach(Tile tile in Selection) {
            if(first) {
                SolidCheckButton.Active = tile.Solid;
                UniSolidCheckButton.Active = tile.UniSolid;
                IceCheckButton.Active = tile.Ice;
                WaterCheckButton.Active = tile.Water;
                SlopeCheckButton.Active = tile.Slope;
                HiddenCheckButton.Active = tile.Hidden;
                DontUseCheckButton.Active = tile.ID == -1;
                DataEntry.Text = tile.Data.ToString();
                AnimFpsEntry.Text = tile.AnimFps.ToString();
                IDEntry.Text = tile.ID.ToString();
                IDEntry.Editable = true;
                first = false;

                if(tile.Images.Count > 0) {
                    nextimage = ((ImageRegion) tile.Images[0]).ImageFile;
                }
            } else {
                IDEntry.Text += "," + tile.ID.ToString();
                IDEntry.Editable = false;
                if(tile.Images.Count > 0
                        && ((ImageRegion) tile.Images[0]).ImageFile != nextimage) {
                    nextimage = "";
                    pixbuf = null;
                }
            }
        }
        if(nextimage != currentimage)
            ChangeImage(nextimage);
        toggling = false;
        DrawingArea.QueueDraw();
    }

    private void FillTileList() {
        TileList.HeadersVisible = true;
        if(TileList.Columns.Length == 0)
            TileList.AppendColumn("Tile", new CellRendererText(), "text", 0);

        ListStore store = new ListStore(typeof(string));

        if(selectedgroup == null) {
            foreach(Tile tile in tileset.Tiles) {
                if(tile == null)
                    continue;
                store.AppendValues(new object[] { tile.ID.ToString() });
            }
        } else {
            foreach(int id in selectedgroup.Tiles) {
                Tile tile = (Tile) tileset.Tiles[id];
                if(tile == null) {
                    Console.WriteLine("tilegroup contains deleted tile");
                    continue;
                }
                store.AppendValues(new object[] { id.ToString() });
            }
        }

        TileList.Model = store;
        TileList.Selection.Mode = SelectionMode.Multiple;
    }

    private void FillTileGroupComboBox() {
        string[] groups = new string[tileset.TileGroups.Count+1];
        groups[0] = "All";

        //Submenu submenu = new Submenu();
        for(int i = 0; i < tileset.TileGroups.Count; ++i) {
            String tilegroup = ((TileGroup) tileset.TileGroups[i]).Name;
            groups[i+1] = tilegroup;
            //submenu.Add(new MenuItem(tilegroup));
        }
        TileGroupComboBox.PopdownStrings = groups;
        TileGroupComboBox.Entry.Editable = false;

        //AddTileGroupMenu.Submenu = submenu;
    }

    private void OnTileGroupComboBoxEntryActivated(object o, EventArgs args) {
        if(TileGroupComboBox.Entry.Text == "All") {
            selectedgroup = null;
        } else {
            foreach(TileGroup tilegroup in tileset.TileGroups) {
                if(tilegroup.Name == TileGroupComboBox.Entry.Text) {
                    selectedgroup = tilegroup;
                    break;
                }
            }
        }
        FillTileList();
    }

    private void OnTileListCursorChanged(object sender, EventArgs e) {
        TreeModel model;
        TreePath[] selectpaths =
            TileList.Selection.GetSelectedRows(out model);

        Selection.Clear();
        foreach(TreePath path in selectpaths) {
            TreeIter iter;
            model.GetIter(out iter, path);
            int id = Int32.Parse(model.GetValue(iter, 0).ToString());
            Selection.Add(tileset.Tiles[id]);
        }
        SelectionChanged();
    }

    private void ShowException(Exception e) {
        MessageDialog dialog = new MessageDialog(MainWindow,
                DialogFlags.Modal | DialogFlags.DestroyWithParent,
                MessageType.Error, ButtonsType.Ok,
                e.Message);
        dialog.Run();
        dialog.Destroy();
    }
}
