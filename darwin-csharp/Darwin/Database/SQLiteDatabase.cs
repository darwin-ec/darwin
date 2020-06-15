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
using Darwin.Matching;
using Darwin.Utilities;

namespace Darwin.Database
{
    // TODO: There's some copy/pasted code in here that could be refactored a little to
    // eliminate some duplication.
    public class SQLiteDatabase : DarwinDatabase
    {
        public const int CurrentDBVersion = 2;

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
            if (version < 2)
                UpgradeToVersion2(conn);
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

                SetVersion(conn, 2);
            }
            catch { }
        }

        // *****************************************************************************
        //
        // Returns complete DatabaseFin<ColorImage>. mDataPos field will be used to map to id in 
        // db for individuals
        //
        public DatabaseFin GetFin(long id)
        {
            DBIndividual individual;
            DBImage image;
            DBOutline outline;
            //DBThumbnail thumbnail;
            DBDamageCategory damagecategory;
            Outline finOutline;
            FloatContour fc = new FloatContour();
            DatabaseFin fin;

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
            image.imagefilename = Path.Combine(new string[] { Options.CurrentUserOptions.CurrentSurveyArea, "catalog", image.imagefilename });

            // assumes list is returned as FIFO (queue)... should be due to use of ORDER BY OrderID
            foreach (var p in points)
            {
                fc.AddPoint(p.xcoordinate, p.ycoordinate);
            }

            finOutline = new Outline(fc);
            finOutline.SetFeaturePoint(FeaturePointType.LeadingEdgeBegin, outline.beginle);
            finOutline.SetFeaturePoint(FeaturePointType.LeadingEdgeEnd, outline.endle);
            finOutline.SetFeaturePoint(FeaturePointType.Notch, outline.notchposition);
            finOutline.SetFeaturePoint(FeaturePointType.Tip, outline.tipposition);
            finOutline.SetFeaturePoint(FeaturePointType.PointOfInflection, outline.endte);
            finOutline.SetLEAngle(0.0, true);

            fin = new DatabaseFin(image.imagefilename,
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

                    DBImage image = new DBImage();
                    image.dateofsighting = fin.DateOfSighting;
                    image.imagefilename = fin.ImageFilename;
                    image.locationcode = fin.LocationCode;
                    image.rollandframe = fin.RollAndFrame;
                    image.shortdescription = fin.ShortDescription;
                    image.fkindividualid = individual.id;
                    InsertImage(conn, ref image);

                    // TODO: Better thumbnail handling?
                    DBThumbnail thumbnail = new DBThumbnail();
                    thumbnail.rows = 1;
                    thumbnail.pixmap = "0";
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

                    // query db as we don't know the image id
                    image = SelectImageByFkIndividualID(individual.id);
                    image.dateofsighting = fin.DateOfSighting;
                    image.imagefilename = fin.ImageFilename;
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
                                orderid = rdr.SafeGetInt("fkOutlineID")
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
                cmd.CommandText = "INSERT INTO Images(ID, ImageFilename, DateOfSighting, RollAndFrame, LocationCode, ShortDescription, fkIndividualID) " +
                    "VALUES (NULL, @ImageFilename, @DateOfSighting, @RollAndFrame, @LocationCode, @ShortDescription, @fkIndividualID);";
                cmd.Parameters.AddWithValue("@ImageFilename", image.imagefilename);
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
                    "DateOfSighting = @DateOfSighting, " +
                    "RollAndFrame = @RollAndFrame, " +
                    "LocationCode = @LocationCode, " +
                    "ShortDescription = @ShortDescription, " +
                    "fkIndividualID = @fkIndividualID " +
                    "WHERE ID = @ID";

                cmd.Parameters.AddWithValue("@ImageFilename", image.imagefilename);
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
                SetVersion(conn, CurrentDBVersion);

                // At this point, the Database class already contains the catalog scheme 
                // specification.  It was set in the Database(...) constructor from 
                // a CatalogScheme passed into the SQLiteDatabase constructor - JHS

                for (int i = 0; i < catalogScheme.CategoryNames.Count; i++)
                {
                    DBDamageCategory cat = new DBDamageCategory
                    {
                        name = catalogScheme.CategoryNames[i],
                        orderid = i
                    };

                    InsertDamageCategory(conn, ref cat);
                }

                // TODO: enter code to populate DBInfo

                conn.Close();
            }
        }
    }
}