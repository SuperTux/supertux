using System;
using System.Collections;
using System.IO;
using System.Drawing;
using Lisp;

public class ImageRegion {
    public String ImageFile;
    public Rectangle Region;
}

public class Tile {
    public int ID;
    public bool Solid;
    public bool UniSolid;
    public bool Ice;
    public bool Water;
    public bool Slope;
    public bool Hidden;
    public bool Hurts;
    public bool FullBox;
    public bool Brick;
    public bool Coin;
    public bool Goal;
    public int NextTile;
    public int Data;
    public float AnimFps;
    public string EditorImage;
    public ArrayList Images = new ArrayList();

    public Tile() {
        ID = -1;
        NextTile = -1;
        AnimFps = 1;
    }

    public void Write(LispWriter writer) {
        writer.StartList("tile");
        writer.Write("id", ID);

        if(Images.Count > 0) {
            writer.StartList("images");
            foreach(ImageRegion region in Images) {
                if(region.Region.Width != 0) {
                    writer.WriteVerbatimLine(
                            String.Format("(region \"{0}\" {1} {2} {3} {4})",
                                region.ImageFile, region.Region.Left,
                                region.Region.Top, region.Region.Width,
                                region.Region.Height));
                } else {
                    writer.WriteVerbatimLine(
                            "\"" + region.ImageFile + "\"");
                }
            }
            writer.EndList("images");
        } else {
            Console.WriteLine("no images on tile " + ID);
        }
        
        if(Solid)
            writer.Write("solid", true);
        if(UniSolid)
            writer.Write("unisolid", true);
        if(Ice)
            writer.Write("ice", true);
        if(Water)
            writer.Write("water", true);
        if(Slope)
            writer.Write("slope-type", Data);
        if(Hurts)
            writer.Write("hurts", true);
        if(Hidden)
            writer.Write("hidden", true);
        if(Coin)
            writer.Write("coin", true);
        if(FullBox)
            writer.Write("fullbox", true);
        if(Brick)
            writer.Write("brick", true);
        if(NextTile >= 0)
            writer.Write("next-tile", NextTile);
        if(Goal)
            writer.Write("goal", true);
        if(EditorImage != null)
            writer.Write("editor-images", EditorImage);
        if(Data != 0)
            writer.Write("data", Data);
        if(Images.Count > 1) {
            if(AnimFps == 1.0)
              AnimFps = 40;
            writer.Write("anim-fps", AnimFps);
        }
        writer.EndList("tile");
    }

    public void Parse(Lisp.Parser parser) {
        int d = parser.Depth;
        while(parser.Parse() && parser.Depth >= d) {
            if(parser.Depth == d+1) {
                if(parser.Type != Parser.LispType.SYMBOL)
                    throw new Exception("expected SYMBOL");
                string symbol = parser.SymbolValue;
                parser.Parse();
                switch(symbol) {
                    case "id":
                        ID = parser.IntegerValue;
                    break;
                    case "images":
                        ParseTileImages(parser);
                        break;
                    case "editor-images":
                        EditorImage = parser.StringValue;
                        break;
                    case "solid":
                        Solid = parser.BoolValue;
                        break;
                    case "unisolid":
                        UniSolid = parser.BoolValue;
                        break;
                    case "ice":
                        Ice = parser.BoolValue;
                        break;
                    case "water":
                        Water = parser.BoolValue;
                        break;
                    case "slope-type":
                        Slope = true;
                        Data = parser.IntegerValue;
                        break;
                    case "anim-fps":
                        AnimFps = parser.FloatValue;
                        break;
                    case "hurts":
                        Hurts = parser.BoolValue;
                        break;
                    case "hidden":
                        Hidden = parser.BoolValue;
                        break;
                    case "data":
                        Data = parser.IntegerValue;
                        break;
                    case "next-tile":
                        NextTile = parser.IntegerValue;
                        break;
                    case "brick":
                        Brick = parser.BoolValue;
                        break;
                    case "fullbox":
                        FullBox = parser.BoolValue;
                        break;
                    case "coin":
                        Coin = parser.BoolValue;
                        break;
                    case "goal":
                        Goal = parser.BoolValue;
                        break;
                    default:
                        Console.WriteLine("Unknown tile element " + symbol);
                        break;
                }
            }
        }
    }

    private void ParseTileImages(Lisp.Parser parser) {
        if(parser.Type == Parser.LispType.END_LIST)
            return;

        int d = parser.Depth;
        do {
            ImageRegion region = new ImageRegion();
            if(parser.Type == Parser.LispType.STRING) {
                region.ImageFile = parser.StringValue;
            } else if(parser.Type == Parser.LispType.START_LIST) {
                ParseImageRegion(parser, region);
            } else {
                throw new Exception("unexpected lisp data: " + parser.Type);
            }
            Images.Add(region);
        } while(parser.Parse() && parser.Depth >= d);
    }

    private void ParseImageRegion(Lisp.Parser parser, ImageRegion region) {
        parser.Parse();
        if(parser.Type != Parser.LispType.SYMBOL)
            throw new Exception("expected symbol");
        if(parser.SymbolValue != "region")
            throw new Exception("expected region symbol");
        parser.Parse();
        if(parser.Type != Parser.LispType.STRING)
            throw new Exception("expected string");
        region.ImageFile = parser.StringValue;

        parser.Parse();
        if(parser.Type != Parser.LispType.INTEGER)
            throw new Exception("expected integer");
        region.Region.X = parser.IntegerValue;

        parser.Parse();
        if(parser.Type != Parser.LispType.INTEGER)
            throw new Exception("expected integer");
        region.Region.Y = parser.IntegerValue;

        parser.Parse();
        if(parser.Type != Parser.LispType.INTEGER)
            throw new Exception("expected integer");
        region.Region.Width = parser.IntegerValue;

        parser.Parse();                                    
        if(parser.Type != Parser.LispType.INTEGER)
            throw new Exception("expected integer");
        region.Region.Height = parser.IntegerValue;

        parser.Parse();
        if(parser.Type != Parser.LispType.END_LIST)
            throw new Exception("expected END_LIST");
    }
}

