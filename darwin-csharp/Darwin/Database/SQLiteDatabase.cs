/*
 * RJ wrote this -- replacing with proper header.
 */

using System;
using System.Collections.Generic;
using System.Data.SQLite;
using System.IO;
using System.Linq;
using System.Text;
using Darwin.Database;
using Darwin.Features;
using Darwin.Matching;
using Darwin.Utilities;

namespace Darwin.Database
{
    // TODO: There's some copy/pasted code in here that could be refactored a little to
    // eliminate some duplication.
    public class SQLiteDatabase : DarwinDatabase
    {
        public const int LatestDBVersion = 3;

        private List<DBDamageCategory> _categories;

        public override List<DBDamageCategory> Categories
        {
            get
            {
                if (_categories == null)
                    _categories = SelectAllDamageCategories();

                return _categories;
            }
        }

        public override List<SelectableDBDamageCategory> SelectableCategories
        {
            get
            {
                return Categories
                    .Select(x => new SelectableDBDamageCategory
                    {
                        IsSelected = true,
                        Name = x.name
                    })
                    .ToList();
            }
        }

        private List<DatabaseFin> _allFins;
        public override List<DatabaseFin> AllFins
        {
            get
            {
                // TODO: The thumbnail part should be temporary
                if (_allFins == null)
                    _allFins = GetAllFins()
                        .Select(x => { x.ThumbnailFilename = x.ImageFilename; return x; })
                        .ToList();

                return _allFins;
            }
        }

        private string _connectionString;

        public SQLiteDatabase(string filename, CatalogScheme cat = null, bool createEmptyDB = false)
        {
            if (string.IsNullOrEmpty(filename))
                throw new ArgumentNullException(nameof(filename));

            Filename = filename;

            // We're using ConnectionStringBuilder to avoid injection attacks
            var builder = new SQLiteConnectionStringBuilder();
            builder.DataSource = filename;
            builder.Version = 3;
            _connectionString = builder.ConnectionString;

            if (!File.Exists(builder.DataSource))
            {
                if (!createEmptyDB)
                    throw new Exception("Database file does not exist, and not trying to create it.");

                SQLiteConnection.CreateFile(builder.DataSource);

                CreateEmptyDatabase(cat);
            }
            else
            {
                // Let's make sure we can open it, and also check the version number and upgrade it if necessary
                using (var conn = new SQLiteConnection(_connectionString))
                {
                    conn.Open();

                    CheckVersionAndUpgrade(conn);

                    conn.Close();
                }
            }
        }

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Security", "CA2100:Review SQL queries for security vulnerabilities", Justification = "<Pending>")]
        private void SetVersion(SQLiteConnection conn, long version)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "PRAGMA user_version = " + version.ToString();
                cmd.ExecuteNonQuery();
            }
        }

        private void CheckVersionAndUpgrade(SQLiteConnection conn)
        {
            long version = 0;
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "PRAGMA user_version;";
                var rdr = cmd.ExecuteReader();

                if (rdr.Read())
                    version = (long)rdr[0];
            }

            // Maybe this should be a little more generic, but just hardcoding version upgrades right now
            if (version < LatestDBVersion)
            {
                if (version < 2)
                    UpgradeToVersion2(conn);

                UpgradeToVersion3(conn);
            }
        }

        private void UpgradeToVersion2(SQLiteConnection conn)
        {
            try
            {
                const string AddScaleToOutlines = "ALTER TABLE Outlines ADD COLUMN Scale REAL DEFAULT NULL";

                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = AddScaleToOutlines;
                    cmd.ExecuteNonQuery();
                }

                const string AddOriginalImageFilename = "ALTER TABLE Images ADD COLUMN OriginalImageFilename TEXT DEFAULT NULL";
                using (var cmd2 = new SQLiteCommand(conn))
                {
                    cmd2.CommandText = AddOriginalImageFilename;
                    cmd2.ExecuteNonQuery();
                }

                SetVersion(conn, 2);
            }
            catch { }
        }


        private void UpgradeToVersion3(SQLiteConnection conn)
        {
            try
            {
                const string CreateFeaturePointsTable = @"CREATE TABLE IF NOT EXISTS OutlineFeaturePoints(
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        Type INTEGER,
                        Position INTEGER,
                        UserSetPosition INTEGER,
                        fkOutlineID INTEGER
                    );
                    CREATE INDEX IF NOT EXISTS IX_OutlineFeaturePoints_fkOutlineID ON OutlineFeaturePoints (fkOutlineID);";

                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = CreateFeaturePointsTable;
                    cmd.ExecuteNonQuery();
                }

                SetVersion(conn, 3);
            }
            catch { }
        }

        // *****************************************************************************
        //
        // Returns complete DatabaseFin<ColorImage>. mDataPos field will be used to map to id in 
        // db for individuals
        //
        public override DatabaseFin GetFin(long id)
        {
            DBIndividual individual;
            DBImage image;
            DBOutline outline;
            //DBThumbnail thumbnail;
            DBDamageCategory damagecategory;
            Outline finOutline;
            FloatContour fc = new FloatContour();

            individual = SelectIndividualByID(id);
            damagecategory = SelectDamageCategoryByID(individual.fkdamagecategoryid);
            image = SelectImageByFkIndividualID(id);
            outline = SelectOutlineByFkIndividualID(id);
            //thumbnail = SelectThumbnailByFkImageID(image.id);
            List<DBPoint> points = SelectPointsByFkOutlineID(outline.id);

            // Although having both of these blocks of code seems uesless, this ensures that
            // the given path contains only the image filename.  If the given path contains
            // more, then the first code block will strip it down.

            // Strip path info
            image.imagefilename = Path.GetFileName(image.imagefilename);

            // Add current path info
            image.imagefilename = Path.Combine(new string[] { "catalog", image.imagefilename });

            // assumes list is returned as FIFO (queue)... should be due to use of ORDER BY OrderID
            foreach (var p in points)
            {
                fc.AddPoint(p.xcoordinate, p.ycoordinate);
            }

            var featurePoints = SelectFeaturePointsByFkOutlineID(outline.id);

            // TODO: The type shouldn't be hardcoded
            if (featurePoints != null && featurePoints.Count > 0)
            {
                var featureSet = FeatureSet.Load(FeatureSetType.DorsalFin, featurePoints);
                finOutline = new Outline(fc, FeatureSetType.DorsalFin, featureSet);
            }
            else
            {
                finOutline = new Outline(fc, FeatureSetType.DorsalFin);
                finOutline.SetFeaturePoint(FeaturePointType.LeadingEdgeBegin, outline.beginle);
                finOutline.SetFeaturePoint(FeaturePointType.LeadingEdgeEnd, outline.endle);
                finOutline.SetFeaturePoint(FeaturePointType.Notch, outline.notchposition);
                finOutline.SetFeaturePoint(FeaturePointType.Tip, outline.tipposition);
                finOutline.SetFeaturePoint(FeaturePointType.PointOfInflection, outline.endte);
            }

            // TODO?
            // finOutline.SetLEAngle(0.0, true);

            DatabaseFin fin = new DatabaseFin(id,
                image.imagefilename,
                finOutline,
                individual.idcode,
                individual.name,
                image.dateofsighting,
                image.rollandframe,
                image.locationcode,
                damagecategory.name,
                image.shortdescription,
                individual.id);

            // TODO: Move/look at constructors
            fin.Scale = outline.scale;
            fin.OriginalImageFilename = image.original_imagefilename;

            if (image != null)
            {
                var imageMods = SelectImageModificationsByFkImageID(image.id);

                var finMods = new List<ImageMod>();

                if (imageMods != null)
                {
                    foreach (var mod in imageMods)
                    {
                        finMods.Add(new ImageMod((ImageModType)mod.operation, mod.value1, mod.value2, mod.value3, mod.value4));
                    }
                }

                fin.ImageMods = finMods;
            }

            return fin;
        }

        // *****************************************************************************
        //
        // Returns all fins from database.
        //
        public override List<DatabaseFin> GetAllFins()
        {
            List<DatabaseFin> fins = new List<DatabaseFin>();

            List<DBIndividual> individuals = SelectAllIndividuals();

            if (individuals == null)
                return fins;

            foreach (var ind in individuals)
            {
                fins.Add(GetFin(ind.id));
            }

            return fins;
        }

        public override long Add(DatabaseFin fin)
        {
            InvalidateAllFins();

            DBDamageCategory dmgCat;
            Outline finOutline;
            FloatContour fc;
            int numPoints;

            //***054 - assume that the image filename contains path
            // information which must be stripped BEFORE saving fin
            fin.ImageFilename = Path.GetFileName(fin.ImageFilename);

            DBIndividual individual = new DBIndividual();

            using (var conn = new SQLiteConnection(_connectionString))
            {
                conn.Open();
                using (var transaction = conn.BeginTransaction())
                {
                    dmgCat = SelectDamageCategoryByName(fin.DamageCategory);

                    if (dmgCat.id == -1)
                        dmgCat = SelectDamageCategoryByName("NONE");

                    individual.idcode = fin.IDCode;
                    individual.name = fin.Name;
                    individual.fkdamagecategoryid = dmgCat.id;
                    InsertIndividual(conn, ref individual);

                    finOutline = fin.FinOutline;

                    DBOutline outline = new DBOutline();
                    outline.scale = fin.Scale;
                    outline.beginle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin);
                    outline.endle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeEnd);
                    outline.notchposition = finOutline.GetFeaturePoint(FeaturePointType.Notch);
                    outline.tipposition = finOutline.GetFeaturePoint(FeaturePointType.Tip);
                    outline.endte = finOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);
                    outline.fkindividualid = individual.id;
                    InsertOutline(conn, ref outline);

                    List<DBPoint> points = new List<DBPoint>();
                    numPoints = finOutline.Length;
                    fc = finOutline.ChainPoints;
                    for (int i = 0; i < numPoints; i++)
                    {
                        points.Add(new DBPoint
                        {
                            xcoordinate = fc[i].X,
                            ycoordinate = fc[i].Y,
                            orderid = i,
                            fkoutlineid = outline.id
                        });
                    }
                    InsertPoints(conn, points);

                    InsertFeaturePoints(conn, outline.id, finOutline.FeatureSet.FeaturePointList);

                    DBImage image = new DBImage();
                    image.dateofsighting = fin.DateOfSighting;
                    image.imagefilename = fin.ImageFilename;
                    image.original_imagefilename = fin.OriginalImageFilename;
                    image.locationcode = fin.LocationCode;
                    image.rollandframe = fin.RollAndFrame;
                    image.shortdescription = fin.ShortDescription;
                    image.fkindividualid = individual.id;
                    InsertImage(conn, ref image);

                    // Fake thumbnail to keep the old version working
                    DBThumbnail thumbnail = new DBThumbnail();
                    thumbnail.rows = 433;
                    thumbnail.pixmap = FakeThumbnail;
                    thumbnail.fkimageid = image.id;
                    InsertThumbnail(conn, ref thumbnail);

                    InsertImageModifications(conn, image.id, fin.ImageMods);

                    transaction.Commit();
                }
                conn.Close();
            }

            return individual.id; // mDataPos field will be used to map to id in db for individuals
        }

        // *****************************************************************************
        //
        // Updates DatabaseFin<ColorImage>
        //
        public override void Update(DatabaseFin fin)
        {
            InvalidateAllFins();

            DBImage image;
            DBOutline outline;
            DBThumbnail thumbnail;
            DBDamageCategory dmgCat;
            FloatContour fc;
            int i, numPoints;

            using (var conn = new SQLiteConnection(_connectionString))
            {
                conn.Open();

                using (var transaction = conn.BeginTransaction())
                {
                    dmgCat = SelectDamageCategoryByName(fin.DamageCategory);

                    DBIndividual individual = new DBIndividual();
                    individual.id = fin.DataPos; // mapping Individuals id to mDataPos
                    individual.idcode = fin.IDCode;
                    individual.name = fin.Name;
                    individual.fkdamagecategoryid = dmgCat.id;
                    UpdateDBIndividual(conn, individual);

                    // we do this as we don't know what the outline id is
                    outline = SelectOutlineByFkIndividualID(individual.id);
                    outline.scale = fin.Scale;
                    outline.beginle = fin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin);
                    outline.endle = fin.FinOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeEnd);
                    outline.notchposition = fin.FinOutline.GetFeaturePoint(FeaturePointType.Notch);
                    outline.tipposition = fin.FinOutline.GetFeaturePoint(FeaturePointType.Tip);
                    outline.endte = fin.FinOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);
                    outline.fkindividualid = individual.id;
                    UpdateOutline(conn, outline);

                    List<DBPoint> points = new List<DBPoint>();
                    numPoints = fin.FinOutline.Length;
                    fc = fin.FinOutline.ChainPoints;

                    for (i = 0; i < numPoints; i++)
                    {
                        points.Add(new DBPoint
                        {
                            xcoordinate = fc[i].X,
                            ycoordinate = fc[i].Y,
                            orderid = i,
                            fkoutlineid = outline.id
                        });
                    }
                    DeletePoints(conn, outline.id);

                    InsertPoints(conn, points);

                    DeleteOutlineFeaturePointsByOutlineID(conn, outline.id);
                    InsertFeaturePoints(conn, outline.id, fin.FinOutline.FeatureSet.FeaturePointList);

                    // query db as we don't know the image id
                    image = SelectImageByFkIndividualID(individual.id);
                    image.dateofsighting = fin.DateOfSighting;
                    image.imagefilename = fin.ImageFilename;
                    image.original_imagefilename = fin.OriginalImageFilename;
                    image.locationcode = fin.LocationCode;
                    image.rollandframe = fin.RollAndFrame;
                    image.shortdescription = fin.ShortDescription;
                    image.fkindividualid = individual.id;
                    UpdateImage(conn, image);

                    // query db as we don't know the thumbnail id
                    thumbnail = SelectThumbnailByFkImageID(image.id);
                    thumbnail.rows = fin.ThumbnailRows;
                    thumbnail.pixmap = new string(fin.ThumbnailPixmap.Cast<char>().ToArray());

                    UpdateThumbnail(conn, thumbnail);

                    DeleteImageModifications(conn, image.id);
                    InsertImageModifications(conn, image.id, fin.ImageMods);

                    transaction.Commit();
                }
                conn.Close();
            }
        }

        public override void UpdateIndividual(DatabaseFin data)
        {
            InvalidateAllFins();

            var dmgCat = SelectDamageCategoryByName(data.DamageCategory);

            DBIndividual individual = new DBIndividual();
            individual.id = data.DataPos; // mapping Individuals id to mDataPos
            individual.idcode = data.IDCode;
            individual.name = data.Name;
            individual.fkdamagecategoryid = dmgCat.id;

            using (var conn = new SQLiteConnection(_connectionString))
            {
                conn.Open();
                UpdateDBIndividual(conn, individual);
                conn.Close();
            }
        }

        // *****************************************************************************
        //
        // Delete fin from database
        //
        public override void Delete(DatabaseFin fin)
        {
            InvalidateAllFins();

            DBOutline outline;
            DBImage image;
            long id;

            // mDataPos field will be used to map to id in db for individuals
            id = fin.DataPos;

            using (var conn = new SQLiteConnection(_connectionString))
            {
                conn.Open();

                using (var transaction = conn.BeginTransaction())
                {
                    outline = SelectOutlineByFkIndividualID(id);
                    image = SelectImageByFkIndividualID(id);

                    DeletePoints(conn, outline.id);
                    DeleteOutlineByFkIndividualID(conn, id);
                    DeleteThumbnailByFkImageID(conn, image.id);
                    DeleteImage(conn, image.id);
                    DeleteIndividual(conn, id);

                    transaction.Commit();
                }

                conn.Close();
            }
        }

        private List<DBIndividual> SelectAllIndividuals()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Individuals;";

                    var rdr = cmd.ExecuteReader();

                    List<DBIndividual> individuals = new List<DBIndividual>();
                    while (rdr.Read())
                    {
                        var individual = new DBIndividual
                        {
                            id = rdr.SafeGetInt("ID"),
                            idcode = rdr.SafeGetString("IDCode"),
                            name = rdr.SafeGetStringStripNone("Name"),
                            fkdamagecategoryid = rdr.SafeGetInt("fkDamageCategoryID")
                        };

                        individuals.Add(individual);
                    }

                    conn.Close();

                    return individuals;
                }
            }
        }

        private DBIndividual SelectIndividualByID(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Individuals WHERE ID = @ID;";
                    cmd.Parameters.AddWithValue("@ID", id);

                    DBIndividual individual = null;
                    using (var rdr = cmd.ExecuteReader())
                    {

                        if (rdr.Read())
                        {
                            individual = new DBIndividual
                            {
                                id = rdr.SafeGetInt("ID"),
                                idcode = rdr.SafeGetString("IDCode"),
                                name = rdr.SafeGetStringStripNone("Name"),
                                fkdamagecategoryid = rdr.SafeGetInt("fkDamageCategoryID")
                            };
                        }
                    }

                    conn.Close();

                    return individual;
                }
            }
        }

        private DBDamageCategory SelectDamageCategoryByName(string name)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories WHERE Name = @Name;";
                    cmd.Parameters.AddWithValue("@Name", name);

                    DBDamageCategory category = null;

                    using (var rdr = cmd.ExecuteReader())
                    {
                        if (rdr.Read())
                        {
                            category = new DBDamageCategory
                            {
                                id = rdr.SafeGetInt("ID"),
                                name = rdr.SafeGetString("Name"),
                                orderid = rdr.SafeGetInt("OrderID")
                            };
                        }
                    }
                    conn.Close();

                    return category;
                }
            }
        }

        private DBDamageCategory SelectDamageCategoryByID(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories WHERE ID = @ID;";
                    cmd.Parameters.AddWithValue("@ID", id);

                    DBDamageCategory category = null;

                    using (var rdr = cmd.ExecuteReader())
                    {
                        if (rdr.Read())
                        {
                            category = new DBDamageCategory
                            {
                                id = rdr.SafeGetInt("ID"),
                                name = rdr.SafeGetString("Name"),
                                orderid = rdr.SafeGetInt("OrderID")
                            };
                        }
                    }

                    conn.Close();

                    return category;
                }
            }
        }

        private List<DBDamageCategory> SelectAllDamageCategories()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories ORDER BY OrderID;";

                    var categories = new List<DBDamageCategory>();

                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var cat = new DBDamageCategory
                            {
                                id = rdr.SafeGetInt("ID"),
                                name = rdr.SafeGetString("Name"),
                                orderid = rdr.SafeGetInt("OrderID")
                            };

                            categories.Add(cat);
                        }

                        conn.Close();
                    }

                    return categories;
                }
            }
        }

        // *****************************************************************************
        //
        // This returns all the DBInfo rows as a list of DBInfo structs.
        //
        public List<DBInfo> SelectAllDBInfo()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DBInfo;";

                    var infos = new List<DBInfo>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var inf = new DBInfo
                            {
                                key = rdr.SafeGetString("Key"),
                                value = rdr.SafeGetString("Value")
                            };

                            infos.Add(inf);
                        }
                    }

                    conn.Close();

                    return infos;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBImageModification> with all rows from 
        // ImageModifications table.
        //
        private List<DBImageModification> SelectAllImageModifications()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM ImageModifications;";

                    var modifications = new List<DBImageModification>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var mod = new DBImageModification
                            {
                                id = rdr.SafeGetInt("ID"),
                                operation = rdr.SafeGetInt("Operation"),
                                value1 = rdr.SafeGetInt("Value1"),
                                value2 = rdr.SafeGetInt("Value2"),
                                value3 = rdr.SafeGetInt("Value3"),
                                value4 = rdr.SafeGetInt("Value4"),
                                orderid = rdr.SafeGetInt("OrderID"),
                                fkimageid = rdr.SafeGetInt("fkImageID")
                            };

                            modifications.Add(mod);
                        }
                    }

                    conn.Close();

                    return modifications;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBImageModification> with all rows from 
        // ImageModifications table where fkImageID equals the given int.
        //
        private List<DBImageModification> SelectImageModificationsByFkImageID(long fkimageid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM ImageModifications WHERE fkImageID = @fkImageID ORDER BY OrderID;";
                    cmd.Parameters.AddWithValue("@fkImageID", fkimageid);

                    var modifications = new List<DBImageModification>();

                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var mod = new DBImageModification
                            {
                                id = rdr.SafeGetInt("ID"),
                                operation = rdr.SafeGetInt("Operation"),
                                value1 = rdr.SafeGetInt("Value1"),
                                value2 = rdr.SafeGetInt("Value2"),
                                value3 = rdr.SafeGetInt("Value3"),
                                value4 = rdr.SafeGetInt("Value4"),
                                orderid = rdr.SafeGetInt("OrderID"),
                                fkimageid = rdr.SafeGetInt("fkImageID")
                            };

                            modifications.Add(mod);
                        }
                    }

                    conn.Close();

                    return modifications;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBImage> with all rows from Images table.
        //
        private List<DBImage> SelectAllImages()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Images;";

                    var images = new List<DBImage>();

                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var img = new DBImage
                            {
                                id = rdr.SafeGetInt("ID"),
                                imagefilename = rdr.SafeGetString("ImageFilename"),
                                original_imagefilename = rdr.SafeGetString("OriginalImageFilename"),
                                dateofsighting = rdr.SafeGetStringStripNone("DateOfSighting"),
                                rollandframe = rdr.SafeGetStringStripNone("RollAndFrame"),
                                locationcode = rdr.SafeGetStringStripNone("LocationCode"),
                                shortdescription = rdr.SafeGetStringStripNone("ShortDescription"),
                                fkindividualid = rdr.SafeGetInt("fkIndividualID")
                            };

                            images.Add(img);
                        }
                    }

                    conn.Close();

                    return images;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBImage> with all rows from Images table where
        // the fkIndividualID equals the given int.
        //
        private List<DBImage> SelectImagesByFkIndividualID(long fkindividualid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Images WHERE fkIndividualID = @fkIndividualID;";
                    cmd.Parameters.AddWithValue("@fkIndividualID", fkindividualid);

                    var images = new List<DBImage>();

                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var img = new DBImage
                            {
                                id = rdr.SafeGetInt("ID"),
                                imagefilename = rdr.SafeGetString("ImageFilename"),
                                original_imagefilename = rdr.SafeGetString("OriginalImageFilename"),
                                dateofsighting = rdr.SafeGetStringStripNone("DateOfSighting"),
                                rollandframe = rdr.SafeGetStringStripNone("RollAndFrame"),
                                locationcode = rdr.SafeGetStringStripNone("LocationCode"),
                                shortdescription = rdr.SafeGetStringStripNone("ShortDescription"),
                                fkindividualid = rdr.SafeGetInt("fkIndividualID")
                            };

                            images.Add(img);
                        }
                    }

                    conn.Close();

                    return images;
                }
            }
        }

        // *****************************************************************************
        //
        // Returns DBImage of row with given fkIndividualID
        //
        private DBImage SelectImageByFkIndividualID(long fkindividualid)
        {
            var images = SelectImagesByFkIndividualID(fkindividualid);

            return images?.FirstOrDefault();
        }

        // *****************************************************************************
        //
        // This returns all the Outlines rows as a list of DBOutline structs.
        //
        private List<DBOutline> selectAllOutlines()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Outlines;";

                    var outlines = new List<DBOutline>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var outline = new DBOutline
                            {
                                id = rdr.SafeGetInt("ID"),
                                scale = rdr.SafeGetDouble("Scale", 1.0),
                                tipposition = rdr.SafeGetInt("TipPosition"),
                                beginle = rdr.SafeGetInt("BeginLE"),
                                endle = rdr.SafeGetInt("EndLE"),
                                notchposition = rdr.SafeGetInt("NotchPosition"),
                                endte = rdr.SafeGetInt("EndTE"),
                                fkindividualid = rdr.SafeGetInt("fkIndividualID")
                            };

                            outlines.Add(outline);
                        }
                    }

                    conn.Close();

                    return outlines;
                }
            }
        }

        // *****************************************************************************
        //
        // Returns DBOutline from Outlines table where the fkIndividualID equals
        // the given int.
        //
        private DBOutline SelectOutlineByFkIndividualID(long fkindividualid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Outlines WHERE fkIndividualID = @fkIndividualID;";
                    cmd.Parameters.AddWithValue("@fkIndividualID", fkindividualid);

                    DBOutline outline = null;
                    using (var rdr = cmd.ExecuteReader())
                    {

                        if (rdr.Read())
                        {
                            outline = new DBOutline
                            {
                                id = rdr.SafeGetInt("ID"),
                                scale = rdr.SafeGetDouble("Scale", 1.0),
                                tipposition = rdr.SafeGetInt("TipPosition"),
                                beginle = rdr.SafeGetInt("BeginLE"),
                                endle = rdr.SafeGetInt("EndLE"),
                                notchposition = rdr.SafeGetInt("NotchPosition"),
                                endte = rdr.SafeGetInt("EndTE"),
                                fkindividualid = rdr.SafeGetInt("fkIndividualID")
                            };
                        }
                    }

                    conn.Close();

                    return outline;
                }
            }
        }

        private List<FeaturePoint> SelectFeaturePointsByFkOutlineID(long fkoutlineid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM OutlineFeaturePoints WHERE fkOutlineID = @fkOutlineID;";
                    cmd.Parameters.AddWithValue("@fkOutlineID", fkoutlineid);

                    List<FeaturePoint> points = new List<FeaturePoint>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var featurePoint = new FeaturePoint
                            {
                                ID = rdr.SafeGetInt("ID"),
                                Type = (FeaturePointType)rdr.SafeGetInt("Type"),
                                Position = rdr.SafeGetInt("Position"),
                                UserSetPosition = rdr.SafeGetInt("UserSetPosition") != 0
                            };

                            points.Add(featurePoint);
                        }
                    }

                    conn.Close();

                    return points;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBPoint> with all rows from Points table where
        // the fkOutlineID equals the given int.
        //
        private List<DBPoint> SelectPointsByFkOutlineID(long fkoutlineid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Points WHERE fkOutlineID = @fkOutlineID ORDER BY OrderID;";
                    cmd.Parameters.AddWithValue("@fkOutlineID", fkoutlineid);

                    List<DBPoint> points = new List<DBPoint>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var point = new DBPoint
                            {
                                id = rdr.SafeGetInt("ID"),
                                xcoordinate = (float)rdr.SafeGetDouble("XCoordinate"),
                                ycoordinate = (float)rdr.SafeGetDouble("YCoordinate"),
                                fkoutlineid = rdr.SafeGetInt("fkOutlineID"),
                                orderid = rdr.SafeGetInt("OrderID")
                            };

                            points.Add(point);
                        }
                    }

                    conn.Close();

                    return points;
                }
            }
        }

        // *****************************************************************************
        //
        // This returns all the Thumbnails rows as a list of DBThumbnail structs.
        //
        private List<DBThumbnail> SelectAllThumbnails()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Thumbnails;";

                    List<DBThumbnail> thumbnails = new List<DBThumbnail>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var thumb = new DBThumbnail
                            {
                                id = rdr.SafeGetInt("ID"),
                                rows = rdr.SafeGetInt("Rows"),
                                pixmap = rdr.SafeGetString("Pixmap"),
                                fkimageid = rdr.SafeGetInt("fkImageID")
                            };

                            thumbnails.Add(thumb);
                        }
                    }

                    conn.Close();

                    return thumbnails;
                }
            }
        }

        // *****************************************************************************
        //
        // Selects a single Thumbnail.
        //
        private DBThumbnail SelectThumbnailByFkImageID(long fkimageid)
        {
            var thumbnails = SelectThumbnailsByFkImageID(fkimageid);

            if (thumbnails == null)
                return null;

            return thumbnails.FirstOrDefault();
        }

        // *****************************************************************************
        //
        // This returns all the Thumbnails rows as a list of DBThumbnail structs.
        //
        private List<DBThumbnail> SelectThumbnailsByFkImageID(long fkimageid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Thumbnails WHERE fkImageID = @fkImageID;";
                    cmd.Parameters.AddWithValue("@fkImageID", fkimageid);

                    List<DBThumbnail> thumbnails = new List<DBThumbnail>();
                    using (var rdr = cmd.ExecuteReader())
                    {
                        while (rdr.Read())
                        {
                            var thumb = new DBThumbnail
                            {
                                id = rdr.SafeGetInt("ID"),
                                rows = rdr.SafeGetInt("Rows"),
                                pixmap = rdr.SafeGetString("Pixmap"),
                                fkimageid = rdr.SafeGetInt("fkImageID")
                            };

                            thumbnails.Add(thumb);
                        }
                    }
                    conn.Close();

                    return thumbnails;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts Individual into Individuals table.  id needs to be unique.
        //
        private long InsertIndividual(SQLiteConnection conn, ref DBIndividual individual)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO Individuals (ID, IDCode, Name, fkDamageCategoryID) " +
                    "VALUES (NULL, @IDCode, @Name, @fkDamageCategoryID);";
                cmd.Parameters.AddWithValue("@IDCode", individual.idcode);
                cmd.Parameters.AddWithValue("@Name", individual.name);
                cmd.Parameters.AddWithValue("@fkDamageCategoryID", individual.fkdamagecategoryid);

                cmd.ExecuteNonQuery();

                individual.id = conn.LastInsertRowId;

                return individual.id;
            }
        }

        // *****************************************************************************
        //
        // Inserts DamageCategory into DamageCategories table.  Ignores id as
        // this is autoincremented in the database.
        //
        private long InsertDamageCategory(SQLiteConnection conn, ref DBDamageCategory damagecategory)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO DamageCategories (ID, Name, OrderID)  " +
                    "VALUES (NULL, @Name, @OrderID);";
                cmd.Parameters.AddWithValue("@Name", damagecategory.name);
                cmd.Parameters.AddWithValue("@OrderID", damagecategory.orderid);

                cmd.ExecuteNonQuery();

                damagecategory.id = conn.LastInsertRowId;

                return damagecategory.id;
            }
        }

        private long InsertFeaturePoint(SQLiteConnection conn, long fkOutlineID, ref FeaturePoint point)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                /*                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        Type INTEGER,
                        Position INTEGER,
                        UserSetPosition INTEGER,*/
                cmd.CommandText = "INSERT INTO OutlineFeaturePoints (ID, Type, Position, UserSetPosition, fkOutlineID) " +
                    "VALUES (NULL, @Type, @Position, @UserSetPosition, @fkOutlineID);";
                cmd.Parameters.AddWithValue("@Type", point.Type);
                cmd.Parameters.AddWithValue("@Position", point.Position);
                cmd.Parameters.AddWithValue("@UserSetPosition", (point.UserSetPosition) ? 1 : 0);
                cmd.Parameters.AddWithValue("@fkOutlineID", fkOutlineID);

                cmd.ExecuteNonQuery();

                point.ID = conn.LastInsertRowId;

                return point.ID;
            }
        }

        // *****************************************************************************
        //
        // Inserts DBPoint into Points table
        //
        private long InsertPoint(SQLiteConnection conn, ref DBPoint point)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO Points (ID, XCoordinate, YCoordinate, fkOutlineID, OrderID) " +
                    "VALUES (NULL, @XCoordinate, @YCoordinate, @fkOutlineID, @OrderID);";
                cmd.Parameters.AddWithValue("@XCoordinate", point.xcoordinate);
                cmd.Parameters.AddWithValue("@YCoordinate", point.ycoordinate);
                cmd.Parameters.AddWithValue("@fkOutlineID", point.fkoutlineid);
                cmd.Parameters.AddWithValue("@OrderID", point.orderid);

                cmd.ExecuteNonQuery();

                point.id = conn.LastInsertRowId;

                return point.id;
            }
        }

        // *****************************************************************************
        //
        // Inserts DBInfo into DBInfo table
        //
        private void InsertDBInfo(SQLiteConnection conn, DBInfo dbinfo)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO DBInfo (Key, Value) " +
                    "VALUES (@Key, @Value);";
                cmd.Parameters.AddWithValue("@Key", dbinfo.key);
                cmd.Parameters.AddWithValue("@Value", dbinfo.value);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Inserts DBOutline into Outlines table
        //
        private long InsertOutline(SQLiteConnection conn, ref DBOutline outline)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO Outlines (ID, Scale, TipPosition, BeginLE, EndLE, NotchPosition, EndTE, fkIndividualID) " +
                    "VALUES (NULL, @Scale, @TipPosition, @BeginLE, @EndLE, @NotchPosition, @EndTE, @fkIndividualID);";

                cmd.Parameters.AddWithValue("@Scale", outline.scale);
                cmd.Parameters.AddWithValue("@TipPosition", outline.tipposition);
                cmd.Parameters.AddWithValue("@BeginLE", outline.beginle);
                cmd.Parameters.AddWithValue("@EndLE", outline.endle);
                cmd.Parameters.AddWithValue("@NotchPosition", outline.notchposition);
                cmd.Parameters.AddWithValue("@EndTE", outline.endte);
                cmd.Parameters.AddWithValue("@fkIndividualID", outline.fkindividualid);

                cmd.ExecuteNonQuery();

                outline.id = conn.LastInsertRowId;

                return outline.id;
            }
        }

        // *****************************************************************************
        //
        // Inserts DBImage into Images table
        //
        private long InsertImage(SQLiteConnection conn, ref DBImage image)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO Images(ID, ImageFilename, OriginalImageFilename, DateOfSighting, RollAndFrame, LocationCode, ShortDescription, fkIndividualID) " +
                    "VALUES (NULL, @ImageFilename, @OriginalImageFilename, @DateOfSighting, @RollAndFrame, @LocationCode, @ShortDescription, @fkIndividualID);";
                cmd.Parameters.AddWithValue("@ImageFilename", image.imagefilename);
                cmd.Parameters.AddWithValue("@OriginalImageFilename", image.original_imagefilename);
                cmd.Parameters.AddWithValue("@DateOfSighting", image.dateofsighting);
                cmd.Parameters.AddWithValue("@RollAndFrame", image.rollandframe);
                cmd.Parameters.AddWithValue("@LocationCode", image.locationcode);
                cmd.Parameters.AddWithValue("@ShortDescription", image.shortdescription);
                cmd.Parameters.AddWithValue("@fkIndividualID", image.fkindividualid);

                cmd.ExecuteNonQuery();

                image.id = conn.LastInsertRowId;

                return image.id;
            }
        }

        private void InsertImageModifications(SQLiteConnection conn, long imageId, List<ImageMod> mods)
        {
            if (mods != null)
            {
                List<DBImageModification> modifications = new List<DBImageModification>();

                for (var j = 0; j < mods.Count; j++)
                {
                    ImageModType modType;
                    int val1, val2, val3, val4;

                    mods[j].Get(out modType, out val1, out val2, out val3, out val4);

                    modifications.Add(new DBImageModification
                    {
                        fkimageid = imageId,
                        operation = (int)modType,
                        value1 = val1,
                        value2 = val2,
                        value3 = val3,
                        value4 = val4,
                        orderid = j + 1
                    });
                }

                InsertImageModifications(conn, modifications);
            }
        }

        // *****************************************************************************
        //
        // Inserts DBImageModification into ImageModifications table
        //
        private long InsertImageModification(SQLiteConnection conn, ref DBImageModification imagemod)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO ImageModifications(ID, Operation, Value1, Value2, Value3, Value4, OrderID, fkImageID) " +
                    "VALUES (NULL, @Operation, @Value1, @Value2, @Value3, @Value4, @OrderID, @fkImageID);";
                cmd.Parameters.AddWithValue("@Operation", imagemod.operation);
                cmd.Parameters.AddWithValue("@Value1", imagemod.value1);
                cmd.Parameters.AddWithValue("@Value2", imagemod.value2);
                cmd.Parameters.AddWithValue("@Value3", imagemod.value3);
                cmd.Parameters.AddWithValue("@Value4", imagemod.value4);
                cmd.Parameters.AddWithValue("@OrderID", imagemod.orderid);
                cmd.Parameters.AddWithValue("@fkImageID", imagemod.fkimageid);

                cmd.ExecuteNonQuery();

                imagemod.id = conn.LastInsertRowId;

                return imagemod.id;
            }
        }

        // *****************************************************************************
        //
        // Inserts DBThumbnail into Thumbnails table
        //
        private long InsertThumbnail(SQLiteConnection conn, ref DBThumbnail thumbnail)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "INSERT INTO Thumbnails (ID, Rows, Pixmap, fkImageID) " +
                    "VALUES (NULL, @Rows, @Pixmap, @fkImageID);";
                cmd.Parameters.AddWithValue("@Rows", thumbnail.rows);
                cmd.Parameters.AddWithValue("@Pixmap", thumbnail.pixmap);
                cmd.Parameters.AddWithValue("@fkImageID", thumbnail.fkimageid);

                cmd.ExecuteNonQuery();

                thumbnail.id = conn.LastInsertRowId;

                return thumbnail.id;
            }
        }

        private void InsertFeaturePoints(SQLiteConnection conn, long fkOutlineID, List<FeaturePoint> points)
        {
            foreach (var p in points)
            {
                var pointCopy = p;
                InsertFeaturePoint(conn, fkOutlineID, ref pointCopy);
            }
        }


        // *****************************************************************************
        //
        // Inserts list of DBPoint's into Points table
        //
        private void InsertPoints(SQLiteConnection conn, List<DBPoint> points)
        {
            foreach (var p in points)
            {
                var pointCopy = p;
                InsertPoint(conn, ref pointCopy);
            }
        }

        // *****************************************************************************
        //
        // Inserts list of DBImageModification's into ImageModifications table
        //
        private void InsertImageModifications(SQLiteConnection conn, List<DBImageModification> imagemods)
        {
            foreach (var i in imagemods)
            {
                var modCopy = i;
                InsertImageModification(conn, ref modCopy);
            }
        }

        // *****************************************************************************
        //
        // Updates outline in Outlines table  
        //
        private void UpdateOutline(SQLiteConnection conn, DBOutline outline)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE Outlines SET " +
                    "Scale = @Scale, " +
                    "TipPosition = @TipPosition, " +
                    "BeginLE = @BeginLE, " +
                    "EndLE = @EndLE, " +
                    "NotchPosition = @NotchPosition, " +
                    "fkIndividualID = @fkIndividualID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@Scale", outline.scale);
                cmd.Parameters.AddWithValue("@TipPosition", outline.tipposition);
                cmd.Parameters.AddWithValue("@BeginLE", outline.beginle);
                cmd.Parameters.AddWithValue("@EndLE", outline.endle);
                cmd.Parameters.AddWithValue("@NotchPosition", outline.notchposition);
                cmd.Parameters.AddWithValue("@fkIndividualID", outline.fkindividualid);
                cmd.Parameters.AddWithValue("@ID", outline.id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Updates row in DamageCategories table using given DBDamageCategory struct.
        // Uses ID field for identifying row.
        //
        private void UpdateDamageCategory(SQLiteConnection conn, DBDamageCategory damagecategory)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE DamageCategories SET " +
                    "Name = @Name, " +
                    "OrderID = @OrderID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@Name", damagecategory.name);
                cmd.Parameters.AddWithValue("@OrderID", damagecategory.orderid);
                cmd.Parameters.AddWithValue("@ID", damagecategory.id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Updates row in Individuals table using given DBIndividual struct.  Uses ID
        // field for identifying row.
        //
        private void UpdateDBIndividual(SQLiteConnection conn, DBIndividual individual)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE Individuals SET " +
                    "IDCode = @IDCode, " +
                    "Name = @Name, " +
                    "fkDamageCategoryID = @fkDamageCategoryID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@IDCode", individual.idcode);
                cmd.Parameters.AddWithValue("@Name", individual.name);
                cmd.Parameters.AddWithValue("@fkDamageCategoryID", individual.fkdamagecategoryid);
                cmd.Parameters.AddWithValue("@ID", individual.id);

                cmd.ExecuteNonQuery();
            }
        }
        // *****************************************************************************
        //
        // Updates row in Images table using given DBImage struct.  Uses ID
        // field for identifying row.
        //
        private void UpdateImage(SQLiteConnection conn, DBImage image)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE Images SET " +
                    "ImageFilename = @ImageFilename, " +
                    "OriginalImageFilename = @OriginalImageFilename, " +
                    "DateOfSighting = @DateOfSighting, " +
                    "RollAndFrame = @RollAndFrame, " +
                    "LocationCode = @LocationCode, " +
                    "ShortDescription = @ShortDescription, " +
                    "fkIndividualID = @fkIndividualID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@ImageFilename", image.imagefilename);
                cmd.Parameters.AddWithValue("@OriginalImageFilename", image.original_imagefilename);
                cmd.Parameters.AddWithValue("@DateOfSighting", image.dateofsighting);
                cmd.Parameters.AddWithValue("@RollAndFrame", image.rollandframe);
                cmd.Parameters.AddWithValue("@LocationCode", image.locationcode);
                cmd.Parameters.AddWithValue("@ShortDescription", image.shortdescription);
                cmd.Parameters.AddWithValue("@fkIndividualID", image.fkindividualid);
                cmd.Parameters.AddWithValue("@ID", image.id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Updates row in ImageModifications table using given DBImageModification
        // struct.  Uses ID field for identifying row.
        //
        private void UpdateImageModification(SQLiteConnection conn, DBImageModification imagemod)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE ImageModifications SET " +
                    "Operation = @Operation, " +
                    "Value1 = @Value1, " +
                    "Value2 = @Value2, " +
                    "Value3 = @Value3, " +
                    "Value4 = @Value4, " +
                    "OrderID = @OrderID, " +
                    "fkImageID = @fkImageID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@Operation", imagemod.operation);
                cmd.Parameters.AddWithValue("@Value1", imagemod.value1);
                cmd.Parameters.AddWithValue("@Value2", imagemod.value2);
                cmd.Parameters.AddWithValue("@Value3", imagemod.value3);
                cmd.Parameters.AddWithValue("@Value4", imagemod.value4);
                cmd.Parameters.AddWithValue("@OrderID", imagemod.orderid);
                cmd.Parameters.AddWithValue("@fkImageID", imagemod.fkimageid);
                cmd.Parameters.AddWithValue("@ID", imagemod.id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Updates row in Thumbnails table using given DBThumbnail
        // struct.  Uses ID field for identifying row.
        //
        private void UpdateThumbnail(SQLiteConnection conn, DBThumbnail thumbnail)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE Thumbnails SET " +
                    "Rows = @Rows, " +
                    "Pixmap = @Pixmap, " +
                    "fkImageID = @fkImageID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@Rows", thumbnail.rows);
                cmd.Parameters.AddWithValue("@Pixmap", thumbnail.pixmap);
                cmd.Parameters.AddWithValue("@fkImageID", thumbnail.fkimageid);
                cmd.Parameters.AddWithValue("@ID", thumbnail.id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Updates row in DBInfo table using given DBInfo
        // struct.  Uses ID field for identifying row.
        //
        private void UpdateDBInfo(SQLiteConnection conn, DBInfo dbinfo)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "UPDATE DBInfo SET " +
                    "Value = @Value " +
                    "WHERE Key = @Key";

                cmd.Parameters.AddWithValue("@Value", dbinfo.value);
                cmd.Parameters.AddWithValue("@Key", dbinfo.key);

                cmd.ExecuteNonQuery();
            }
        }

        private void DeleteOutlineFeaturePointsByOutlineID(SQLiteConnection conn, long fkOutlineID)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM OutlineFeaturePoints WHERE fkOutlineID = @ID";
                cmd.Parameters.AddWithValue("@ID", fkOutlineID);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Deletes set of points from Points table using fkOutlineID  
        //
        private void DeletePoints(SQLiteConnection conn, long fkOutlineID)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Points WHERE fkOutlineID = @ID";
                cmd.Parameters.AddWithValue("@ID", fkOutlineID);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete outline from Outlines table using fkIndividualID  
        //
        private void DeleteOutlineByFkIndividualID(SQLiteConnection conn, long fkIndividualID)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Outlines WHERE fkIndividualID = @ID";
                cmd.Parameters.AddWithValue("@ID", fkIndividualID);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete outline from Outlines table using id  
        //
        private void DeleteOutlineByID(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM Outlines WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete individual from Individuals table using id  
        //
        private void DeleteIndividual(SQLiteConnection conn, long id)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Individuals WHERE ID = @ID";
                cmd.Parameters.AddWithValue("@ID", id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete damagecategory from DamageCategories table using id  
        //
        private void DeleteDamageCategory(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM DamageCategories WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete image from Images table using id  
        //
        private void DeleteImage(SQLiteConnection conn, long id)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Images WHERE ID = @ID";
                cmd.Parameters.AddWithValue("@ID", id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete imagemod from ImageModifications table using id  
        //
        private void DeleteImageModification(SQLiteConnection conn, int id)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM ImageModifications WHERE ID = @ID";
                cmd.Parameters.AddWithValue("@ID", id);

                cmd.ExecuteNonQuery();
            }
        }

        private void DeleteImageModifications(SQLiteConnection conn, long fkImageID)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM ImageModifications WHERE fkImageID = @fkImageID";
                cmd.Parameters.AddWithValue("@fkImageID", fkImageID);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete thumbnail from Thumbnails table using id  
        //
        private void DeleteThumbnail(SQLiteConnection conn, int id)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Thumbnails WHERE ID = @ID";
                cmd.Parameters.AddWithValue("@ID", id);

                cmd.ExecuteNonQuery();
            }
        }

        // *****************************************************************************
        //
        // Delete thumbnail from Thumbnails table using fkImageID  
        //
        private void DeleteThumbnailByFkImageID(SQLiteConnection conn, long id)
        {
            using (var cmd = new SQLiteCommand(conn))
            {
                cmd.CommandText = "DELETE FROM Thumbnails WHERE fkImageID = @ID";
                cmd.Parameters.AddWithValue("@ID", id);

                cmd.ExecuteNonQuery();
            }
        }

        private void InvalidateAllFins()
        {
            _allFins = null;
        }

        public override void CreateEmptyDatabase(CatalogScheme catalogScheme)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                // SQL CREATE TABLE statements... might be better off defined in the header as a constant..
                string tableCreate = @"CREATE TABLE IF NOT EXISTS DamageCategories (
                    ID INTEGER PRIMARY KEY AUTOINCREMENT, 
                    OrderID INTEGER, 
                    Name TEXT);

                    CREATE TABLE IF NOT EXISTS Individuals (
                        ID INTEGER PRIMARY KEY,
                        IDCode TEXT,
                        Name TEXT,
                        fkDamageCategoryID INTEGER
                    );

                    CREATE TABLE IF NOT EXISTS Images (
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        fkIndividualID INTEGER,
                        ImageFilename TEXT,
                        OriginalImageFilename TEXT DEFAULT NULL,
                        DateOfSighting TEXT,
                        RollAndFrame TEXT,
                        LocationCode TEXT,
                        ShortDescription TEXT
                    );

                    CREATE TABLE IF NOT EXISTS ImageModifications ( 
                        ID INTEGER PRIMARY KEY AUTOINCREMENT, 
                        Operation INTEGER, 
                        Value1 INTEGER, 
                        Value2 INTEGER, 
                        Value3 INTEGER, 
                        Value4 INTEGER, 
                        OrderID INTEGER, 
                        fkImageID INTEGER
                    );

                    CREATE TABLE IF NOT EXISTS Thumbnails (
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        fkImageID INTEGER,
                        Rows INTEGER, 
                        Pixmap TEXT 
                    );

                    CREATE TABLE IF NOT EXISTS Outlines (
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        Scale REAL DEFAULT NULL,
                        TipPosition INTEGER,
                        BeginLE INTEGER,
                        EndLE INTEGER,
                        NotchPosition INTEGER,
                        EndTE INTEGER,
                        fkIndividualID INTEGER
                    );

                    CREATE TABLE IF NOT EXISTS OutlineFeaturePoints (
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        Type INTEGER,
                        Position INTEGER,
                        UserSetPosition INTEGER,
                        fkOutlineID INTEGER
                    );

                    CREATE TABLE IF NOT EXISTS Points (
                        ID INTEGER PRIMARY KEY AUTOINCREMENT,
                        XCoordinate REAL,
                        YCoordinate REAL,
                        fkOutlineID INTEGER,
                        OrderID INTEGER
                    );

                    CREATE INDEX IF NOT EXISTS dmgcat_orderid ON DamageCategories (OrderID);

                    CREATE INDEX IF NOT EXISTS dmgcat_name ON DamageCategories (Name);

                    CREATE INDEX IF NOT EXISTS imgmod_img ON  ImageModifications (fkImageID);

                    CREATE INDEX IF NOT EXISTS img_indiv ON Images (fkIndividualID);

                    CREATE INDEX IF NOT EXISTS outln_indiv ON Outlines (fkIndividualID);

                    CREATE INDEX IF NOT EXISTS IX_OutlineFeaturePoints_fkOutlineID ON OutlineFeaturePoints (fkOutlineID);

                    CREATE INDEX IF NOT EXISTS pts_outln ON Points (fkOutlineID);

                    CREATE INDEX IF NOT EXISTS pts_order ON Points (OrderID);

                    CREATE INDEX IF NOT EXISTS pts_outln_order ON Points (fkOutlineID, OrderID);

                    CREATE INDEX IF NOT EXISTS thmbnl_img ON Thumbnails (fkImageID);";

                conn.Open();

                using (SQLiteCommand cmd = new SQLiteCommand(conn))
                {
                    var transaction = conn.BeginTransaction();

                    cmd.CommandText = tableCreate;
                    cmd.ExecuteNonQuery();

                    transaction.Commit();
                }

                // Set the DB versioning to the latest
                SetVersion(conn, LatestDBVersion);

                // At this point, the Database class already contains the catalog scheme 
                // specification.  It was set in the Database(...) constructor from 
                // a CatalogScheme passed into the SQLiteDatabase constructor - JHS

                for (int i = 0; i < catalogScheme.Categories.Count; i++)
                {
                    DBDamageCategory cat = new DBDamageCategory
                    {
                        name = catalogScheme.Categories[i].Name,
                        orderid = i
                    };

                    InsertDamageCategory(conn, ref cat);
                }

                // TODO: enter code to populate DBInfo

                conn.Close();
            }
        }

        private const string FakeThumbnail = @"25 25 407 2
!! c #000000
!# c #4A5D6B
!$ c #5E7F86
!% c #9CC7AC
!& c #C0DCD0
!'' c #E7FBF2
!(c #F2FBF6
!) c #FEFFFF
!* c #FEFFFA
!+ c #F4FFEF
!, c #FEFEFC
!- c #FDFDFB
!. c #EFFDEE
!/ c #F3FFE6
!0 c #C9EECF
!1 c #AADACE
!2 c #98BEA5
!3 c #AFD9CD
!4 c #CCE2CB
!5 c #C5EFD7
!6 c #C3E5D7
!7 c #C0E0DB
!8 c #B4DFB2
!9 c #A2C9AE
!: c #B1D0BE
!; c #878B8E
!< c #839B83
!= c #84A4A3
!> c #739887
!? c #7A9C81
!@ c #B1BEB7
!A c #ADC2AF
!B c #DAFBF2
!C c #DCFBD9
!D c #C5FADE
!E c #F1FCEE
!F c #EEFDE6
!G c #EBFFF3
!H c #C4E2BC
!I c #A5CFAB
!J c #BEFADE
!K c #D9FFDB
!L c #E3FFF3
!M c #F1FFFA
!N c #FBFDEF
!O c #F6FFFD
!P c #FBFDF0
!Q c #F6FFFC
!R c #F1FFF9
!S c #8EACAC
!T c #A3B5A9
!U c #ABC6BF
!V c #C1D7CA
!W c #E1F1D4
!X c #E8FFEA
!Y c #DEFAE1
!Z c #B9E0CE
![c #8BBBA1
!\ c #82B094
!]
        c #7AA47C
!^ c #88B29E
!_ c #BADEC4
!` c #EEFFED
!a c #FDFDFD
!b c #FAFFF6
!c c #B283A5
!d c #160D3A
!e c #0B0231
!f c #0D042F
!g c #0A052E
!h c #FFFFFB
!i c #FEFFF5
!j c #A9C5B7
!k c #ABC7B8
!l c #E7F7DC
!m c #CBF3D9
!n c #799A89
!o c #56716A
!p c #5B7F59
!q c #486B4A
!r c #56794F
!s c #668971
!t c #6D948F
!u c #6D9477
!v c #7CA793
!w c #93B5A4
!x c #BCDACE
!y c #CEF8E2
!z c #CBC2B1
!{ c #130A3F
!| c #08012D
!}
    c #070127
!~c #050126
! c #0D022D
#! c #B2DBB9
## c #DAFBCC
#$ c #A8AC9E
#% c #B4CFD6
#& c #99BEAD
#'' c #AAD8BB
#( c #C5E7D9
#) c #D7F3DD
#* c #D0F1DE
#+ c #B9DBDC
#, c #B9DAD3
#- c #BEDCDA
#. c #A4CCC3
#/ c #84B6B7
#0 c #90B594
#1 c #7EA391
#2 c #7AA87A
#3 c #2A1D53
#4 c #07002C
#5 c #060223
#6 c #05021F
#7 c #03001F
#8 c #060121
#9 c #050122
#: c #CBFCCF
#; c #E2FAE2
#< c #DEFFFB
#= c #A6C3CB
#> c #97B8B1
#? c #A0BCB0
#@ c #ACD1C0
#A c #D7E9E9
#B c #F0FDEC
#C c #D3F1D7
#D c #E5F4DD
#E c #E5FAE9
#F c #E0F7E3
#G c #D9EEDF
#H c #D0E4C8
#I c #E4FDDD
#J c #D2E6CA
#K c #0F023A
#L c #030024
#M c #060024
#N c #050124
#O c #231B40
#P c #D4E9DA
#Q c #D3F1D5
#R c #D9F1D7
#S c #A4C9A0
#T c #D5E7D7
#U c #E9FEEB
#V c #FCFFEE
#W c #F7FEEC
#X c #DEF9EA
#Y c #DEF2CD
#Z c #CAE5BC
#[ c #EDF3D1
#\ c #E9FFEE
#] c #E7FCED
#^ c #D1ECE7
#_ c #DAF6DD
#` c #09032F
#a c #0B0430
#b c #040021
#c c #050221
#d c #070029
#e c #E3FAF0
#f c #EEFFF7
#g c #DCFDE2
#h c #DAFCE4
#i c #C0DABD
#j c #D1DFCE
#k c #C4DCDC
#l c #B5D4C5
#m c #BADFD7
#n c #95ACA2
#o c #86A898
#p c #8FA895
#q c #678778
#r c #789D8B
#s c #A7D0B2
#t c #CAE4CB
#u c #080031
#v c #060029
#w c #060028
#x c #060022
#y c #050021
#z c #06011F
#{ c #91A5A3
#| c #FEFFF7
#} c #FEFFF9
#~ c #FEFEFE
# c #FFFFFD
$! c #9FB0A8
$# c #768888
$$ c #6F9073
$% c #81A192
$& c #91B99E
$'' c #99C19F
$(c #D7F0D3
$) c #DAF7DB
$* c #EEFFEA
$+ c #F6FFFB
$, c #EBFFF0
$- c #0B002D
$. c #060026
$/ c #DAF5CA
$0 c #E7FEEE
$1 c #E5FCE2
$2 c #EAFDE1
$3 c #E7FFE4
$4 c #CEE9E0
$5 c #DDF5E7
$6 c #D5E4CF
$7 c #BCE3D1
$8 c #B8CECC
$9 c #AAD0D1
$: c #92B7AF
$; c #83B3A5
$< c #9DAA99
$= c #8EB19D
$> c #0C013D
$? c #090329
$@ c #07002E
$A c #040020
$B c #070123
$C c #E2F8E3
$D c #E0FBD8
$E c #F0FFF4
$F c #E2FBE5
$G c #C6E2D3
$H c #90A499
$I c #98B5B1
$J c #789C80
$K c #70807F
$L c #87A186
$M c #A7CFC7
$N c #C8EED7
$O c #C2D3C0
$P c #D1EBE0
$Q c #130238
$R c #090327
$S c #030325
$T c #060129
$U c #06002C
$V c #030126
$W c #BBDDBC
$X c #BFE8CA
$Y c #C0E5BC
$Z c #A9CEAC
$[c #84B190
$\ c #A4C9AA
$]
    c #99A4B8
$^ c #C0D7C7
$_ c #CDEBE3
$` c #C7DBBF
$a c #B6DABE
$b c #B1D9BF
$c c #C7DED6
$d c #DDF7EC
$e c #15063F
$f c #060227
$g c #04001D
$h c #04001B
$i c #070026
$j c #B7D0B0
$k c #FFF9F2
$l c #ABC8AA
$m c #FAFFFF
$n c #68985E
$o c #D0E8DB
$p c #C7E1D4
$q c #C5E0CF
$r c #CBE3D5
$s c #C6E8DA
$t c #D1EAE4
$u c #BFE4C5
$v c #0E0137
$w c #060225
$x c #04001F
$y c #2F2B42
$z c #98BD9B
${ c #A4C5AA
$| c #9DBEAB
$}
c #F4F4F2
$~c #6B817E
$ c #607475
%! c #6B886A
%# c #647F90
%$ c #76A07A
%% c #7F8981
%& c #0B0133
%'' c #0C0227
%(c #050020
%) c #040229
%* c #05002B
%+ c #08012B
%, c #261839
%- c #588252
%. c #7DA07F
%/ c #73A08C
%0 c #688F5A
%1 c #587771
%2 c #475258
%3 c #5F815C
%4 c #506B5C
%5 c #667467
%6 c #0A0231
%7 c #040120
%8 c #06021D
%9 c #050025
%: c #06012A
%; c #040322
%< c #070125
%= c #86816D
%> c #689368
%? c #94B48F
%@ c #68895C
%A c #557357
%B c #375C64
%C c #466659
%D c #3D574C
%E c #0D023E
%F c #040027
%G c #050028
%H c #050029
%I c #040028
%J c #1A0838
%K c #D5C496
%L c #A0C2AA
%M c #B5D1A8
%N c #4A5456
%O c #395B43
%P c #567150
%Q c #070030
%R c #07031E
%S c #07012F
%T c #070033
%U c #090029
%V c #190835
%W c #1E0839
%X c #220931
%Y c #2C1438
%Z c #351A39
%[c #CCA878
%\ c #516A67
%] c #738C88
%^ c #1F0A4B
%_ c #10002F
%` c #090031
%a c #170338
%b c #1C0A32
%c c #1C0533
%d c #150B47
%e c #11052D
%f c #07022B
%g c #05002F
%h c #412244
%i c #41183A
%j c #3D2034
%k c #55254F
%l c #522E48
%m c #5F3751
%n c #6D4C61
%o c #6F7978
%p c #1F0737
%q c #271135
%r c #18012D
%s c #1B0128
%t c #290F32
%u c #28113F
%v c #220C32
%w c #21002B
%x c #351C53
%y c #35194C
%z c #3A1646
%{ c #320C47
%| c #030221
%} c #0A012C
%~ c #130127
% c #371838
&! c #93749D
&# c #A39185
&$ c #4E2A38
&% c #6A4264
&& c #8D63A2
&'' c #663855
&( c #37143E
&) c #270A36
&* c #1B0931
&+ c #281040
&, c #1D0535
&- c #1E0235
&. c #260A33
&/ c #220F3D
&0 c #1D0F42
&1 c #290A43
&2 c #4E2A4E
&3 c #41264F
&4 c #A0839F
&5 c #2E133C
&6 c #3E0E42
&7 c #DAE4E6
&8 c #EAD9C7
&9 c #FAFFD7
&: c #573454
&; c #AD82DB
&< c #533453
&= c #5B3564
&> c #714A5F
&? c #877680
&@ c #5F4A71
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!#!$!%!&!''!(!)!*!+!,!*!-!.!/!0!1!2!3!4!5!6!7!8!9!:
!;!<!=!>!?!@!A!B!C!D!E!F!)!G!H!I!J!K!L!M!N!O!P!Q!R
!S!T!U!V!W!X!Y!Z![!\!]!^!_!`!a!,!*!b!c!d!e!f!g!h!i
!j!k!l!m!n!o!p!q!r!s!t!u!v!w!x!y!z!{!|!}!~!~!#!##
#$#%#&#''#(#)#*#+#,#-#.#/#0#1#2#3#4#5#6#7#8#9#:#;#<
#=#>#?#@#A#B#C#D#E#F#G#H#I#J#K#4#L#M#9#9#N#O#P#Q#R
#S#T#U#V#W#X#Y#Z#[#\#]#^#_#`#a!}!~#b#c#6#d#e#f#g#h
#i#j#k#l#m#n#o#p#q#r#s#t#u#v#w#d#x#y#z#x#{#|#}#~#
$!$#$$$%$&$''$($)$*$+$,$-#4#d$.!~#9#8#M#z$/$0$1$2$3
$4$5$6$7$8$9$:$;$<$=$>$?$@$A#9#9#9#M$.$B$C$D$E$F$G
$H$I$J$K$L$M$N$O$P$Q$R$S$T#9#9$U#9!}$V$W$X$Y$Z$[$\
$]$^$_$`$a$b$c$d$e$.#d$B#z#M#8$f$g#8$h$i$j$k$l$m$n
$o$p$q$r$s$t$u$v#d#M!}!~#8#9#9$w#x#d$x#z$y$z${$|$}
$~$%!%#%$%%%&%''$T$B#d$.#x!}#b%(%)%*$B%+%,%-%.%/%0
%1%2%3%4%5%6#8%7%8!}#y!}%9$T#y%:!~#N%;#L%<%=%>%?%@
%A%B%C%D%E!~#N!~#c$T%F%G!~%H$T$U%G#9#c%I#d%J%K%L%M
%N%O%P%Q%R!~!~#9#7!}#N$f$f%S%I%T$f!~%U%V%W%X%Y%Z%[
%\%]%^%_%`%a%b%c%d%e$.!~%f%g$w%H#9%:%h%i%j%k%l%m%n
%o%p%q%r%s%t%u%v%w%x%y%z%{%|#5%:%}%~%&!&#&$&%&&&''
&(&)&*&+&,&-&.&/&0&1&2&3&4&5&6&7&8&9&:&;&<&=&>&?&@
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
";
    }
}