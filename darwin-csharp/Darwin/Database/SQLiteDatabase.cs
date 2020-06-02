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
using Darwin.Utilities;

namespace Darwin.Database
{
    // TODO: There's some copy/pasted code in here that could be refactored a little to
    // eliminate some duplication.
    public class SQLiteDatabase : DarwinDatabase
    {
        private string _connectionString;

        public SQLiteDatabase(string filename, Options options, CatalogScheme cat, bool createEmptyDB)
        {
            if (string.IsNullOrEmpty(filename))
                throw new ArgumentNullException(nameof(filename));

            if (options == null)
                throw new ArgumentNullException(nameof(options));

            mFilename = filename;

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
            }

            // Let's make sure we can open it
            // We're using all synchronous code, and we're going to rely on connection pooling,
            // so we're going to try opening/closing the connection as quickly as possible throughout.
            using (var conn = new SQLiteConnection(_connectionString))
            {
                conn.Open();

                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "pragma quick_check;";
                    cmd.ExecuteNonQuery();
                }

                conn.Close();
            }

            loadLists();
        }

        public List<DBIndividual> selectAllIndividuals()
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
                            name = rdr.SafeGetString("Name"),
                            fkdamagecategoryid = rdr.SafeGetInt("fkDamageCategoryID")
                        };

                        individuals.Add(individual);
                    }

                    conn.Close();

                    return individuals;
                }
            }
        }

        public DBIndividual selectIndividualByID(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Individuals WHERE ID = @ID;";
                    cmd.Parameters.AddWithValue("@ID", id);

                    var rdr = cmd.ExecuteReader();

                    DBIndividual individual = null;
                    if (rdr.Read())
                    {
                        individual = new DBIndividual
                        {
                            id = rdr.SafeGetInt("ID"),
                            idcode = rdr.SafeGetString("IDCode"),
                            name = rdr.SafeGetString("Name"),
                            fkdamagecategoryid = rdr.SafeGetInt("fkDamageCategoryID")
                        };
                    }

                    conn.Close();

                    return individual;
                }
            }
        }

        public DBDamageCategory selectDamageCategoryByName(string name)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories WHERE Name = '@Name';";
                    cmd.Parameters.AddWithValue("@Name", name);

                    var rdr = cmd.ExecuteReader();

                    DBDamageCategory category = null;

                    if (rdr.Read())
                    {
                        category = new DBDamageCategory
                        {
                            id = rdr.SafeGetInt("ID"),
                            name = rdr.SafeGetString("Name"),
                            orderid = rdr.SafeGetInt("OrderID")
                        };
                    }

                    conn.Close();

                    return category;
                }
            }
        }

        public DBDamageCategory selectDamageCategoryByID(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories WHERE ID = @ID;";
                    cmd.Parameters.AddWithValue("@ID", id);

                    var rdr = cmd.ExecuteReader();

                    DBDamageCategory category = null;

                    if (rdr.Read())
                    {
                        category = new DBDamageCategory
                        {
                            id = rdr.SafeGetInt("ID"),
                            name = rdr.SafeGetString("Name"),
                            orderid = rdr.SafeGetInt("OrderID")
                        };
                    }

                    conn.Close();

                    return category;
                }
            }
        }

        public List<DBDamageCategory> selectAllDamageCategories()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DamageCategories ORDER BY OrderID;";

                    var rdr = cmd.ExecuteReader();

                    var categories = new List<DBDamageCategory>();

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

                    return categories;
                }
            }
        }

        // *****************************************************************************
        //
        // This returns all the DBInfo rows as a list of DBInfo structs.
        //
        public List<DBInfo> selectAllDBInfo()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM DBInfo;";

                    var rdr = cmd.ExecuteReader();

                    var infos = new List<DBInfo>();

                    while (rdr.Read())
                    {
                        var inf = new DBInfo
                        {
                            key = rdr.SafeGetString("Key"),
                            value = rdr.SafeGetString("Value")
                        };

                        infos.Add(inf);
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
        public List<DBImageModification> selectAllImageModifications()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM ImageModifications;";

                    var rdr = cmd.ExecuteReader();

                    var modifications = new List<DBImageModification>();

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
        public List<DBImageModification> selectImageModificationsByFkImageID(int fkimageid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM ImageModifications WHERE fkImageID = @fkImageID;";
                    cmd.Parameters.AddWithValue("@fkImageID", fkimageid);

                    var rdr = cmd.ExecuteReader();

                    var modifications = new List<DBImageModification>();

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

                    conn.Close();

                    return modifications;
                }
            }
        }

        // *****************************************************************************
        //
        // Populates given list<DBImage> with all rows from Images table.
        //
        public List<DBImage> selectAllImages()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Images;";

                    var rdr = cmd.ExecuteReader();

                    var images = new List<DBImage>();

                    while (rdr.Read())
                    {
                        var img = new DBImage
                        {
                            id = rdr.SafeGetInt("ID"),
                            imagefilename = rdr.SafeGetString("ImageFilename"),
                            dateofsighting = rdr.SafeGetString("DateOfSighting"),
                            rollandframe = rdr.SafeGetString("RollAndFrame"),
                            locationcode = rdr.SafeGetString("LocationCode"),
                            shortdescription = rdr.SafeGetString("ShortDescription"),
                            fkindividualid = rdr.SafeGetInt("fkIndividualID")
                        };

                        images.Add(img);
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
        public List<DBImage> selectImagesByFkIndividualID(long fkindividualid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Images WHERE fkIndividualID = @fkIndividualID;";
                    cmd.Parameters.AddWithValue("@fkIndividualID", fkindividualid);

                    var rdr = cmd.ExecuteReader();

                    var images = new List<DBImage>();

                    while (rdr.Read())
                    {
                        var img = new DBImage
                        {
                            id = rdr.SafeGetInt("ID"),
                            imagefilename = rdr.SafeGetString("ImageFilename"),
                            dateofsighting = rdr.SafeGetString("DateOfSighting"),
                            rollandframe = rdr.SafeGetString("RollAndFrame"),
                            locationcode = rdr.SafeGetString("LocationCode"),
                            shortdescription = rdr.SafeGetString("ShortDescription"),
                            fkindividualid = rdr.SafeGetInt("fkIndividualID")
                        };

                        images.Add(img);
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
        public DBImage selectImageByFkIndividualID(long fkindividualid)
        {
            var images = selectImagesByFkIndividualID(fkindividualid);

            return images?.FirstOrDefault();
        }

        // *****************************************************************************
        //
        // This returns all the Outlines rows as a list of DBOutline structs.
        //
        public List<DBOutline> selectAllOutlines()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Outlines;";

                    var rdr = cmd.ExecuteReader();

                    var outlines = new List<DBOutline>();

                    while (rdr.Read())
                    {
                        var outline = new DBOutline
                        {
                            id = rdr.SafeGetInt("ID"),
                            tipposition = rdr.SafeGetInt("TipPosition"),
                            beginle = rdr.SafeGetInt("BeginLE"),
                            endle = rdr.SafeGetInt("EndLE"),
                            notchposition = rdr.SafeGetInt("NotchPosition"),
                            endte = rdr.SafeGetInt("EndTE"),
                            fkindividualid = rdr.SafeGetInt("fkIndividualID")
                        };

                        outlines.Add(outline);
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
        public DBOutline selectOutlineByFkIndividualID(long fkindividualid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Outlines WHERE fkIndividualID = @fkIndividualID;";
                    cmd.Parameters.AddWithValue("@fkIndividualID", fkindividualid);

                    var rdr = cmd.ExecuteReader();

                    DBOutline outline = null;

                    if (rdr.Read())
                    {
                        outline = new DBOutline
                        {
                            id = rdr.SafeGetInt("ID"),
                            tipposition = rdr.SafeGetInt("TipPosition"),
                            beginle = rdr.SafeGetInt("BeginLE"),
                            endle = rdr.SafeGetInt("EndLE"),
                            notchposition = rdr.SafeGetInt("NotchPosition"),
                            endte = rdr.SafeGetInt("EndTE"),
                            fkindividualid = rdr.SafeGetInt("fkIndividualID")
                        };
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
        public List<DBPoint> selectPointsByFkOutlineID(long fkoutlineid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Points WHERE fkOutlineID = @fkOutlineID ORDER BY OrderID;";
                    cmd.Parameters.AddWithValue("@fkOutlineID", fkoutlineid);

                    var rdr = cmd.ExecuteReader();

                    List<DBPoint> points = new List<DBPoint>();

                    while (rdr.Read())
                    {
                        var point = new DBPoint
                        {
                            id = rdr.SafeGetInt("ID"),
                            xcoordinate = rdr.SafeGetInt("XCoordinate"),
                            ycoordinate = rdr.SafeGetInt("YCoordinate"),
                            fkoutlineid = rdr.SafeGetInt("fkOutlineID"),
                            orderid = rdr.SafeGetInt("fkOutlineID")
                        };

                        points.Add(point);
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
        public List<DBThumbnail> selectAllThumbnails()
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Thumbnails;";

                    var rdr = cmd.ExecuteReader();

                    List<DBThumbnail> thumbnails = new List<DBThumbnail>();

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

                    conn.Close();

                    return thumbnails;
                }
            }
        }

        // *****************************************************************************
        //
        // Selects a single Thumbnail.
        //
        public DBThumbnail selectThumbnailByFkImageID(long fkimageid)
        {
            var thumbnails = selectThumbnailsByFkImageID(fkimageid);

            if (thumbnails == null)
                return null;

            return thumbnails.FirstOrDefault();
        }

        // *****************************************************************************
        //
        // This returns all the Thumbnails rows as a list of DBThumbnail structs.
        //
        public List<DBThumbnail> selectThumbnailsByFkImageID(long fkimageid)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "SELECT * FROM Thumbnails WHERE fkImageID = @fkImageID;";
                    cmd.Parameters.AddWithValue("@fkImageID", fkimageid);

                    var rdr = cmd.ExecuteReader();

                    List<DBThumbnail> thumbnails = new List<DBThumbnail>();

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

                    conn.Close();

                    return thumbnails;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts Individual into Individuals table.  id needs to be unique.
        //
        public long insertIndividual(ref DBIndividual individual)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO Individuals (ID, IDCode, Name, fkDamageCategoryID) " +
                        "VALUES (NULL, '@IDCode', '@Name', @fkDamageCategoryID);";
                    cmd.Parameters.AddWithValue("@IDCode", individual.idcode);
                    cmd.Parameters.AddWithValue("@Name", individual.name);
                    cmd.Parameters.AddWithValue("@fkDamageCategoryID", individual.fkdamagecategoryid);

                    cmd.ExecuteNonQuery();

                    individual.id = conn.LastInsertRowId;

                    conn.Close();

                    return individual.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DamageCategory into DamageCategories table.  Ignores id as
        // this is autoincremented in the database.
        //
        public long insertDamageCategory(ref DBDamageCategory damagecategory)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO DamageCategories (ID, Name, OrderID)  " +
                        "VALUES (NULL, '@Name', '@OrderID');";
                    cmd.Parameters.AddWithValue("@Name", damagecategory.name);
                    cmd.Parameters.AddWithValue("@OrderID", damagecategory.orderid);

                    cmd.ExecuteNonQuery();

                    damagecategory.id = conn.LastInsertRowId;

                    conn.Close();

                    return damagecategory.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBPoint into Points table
        //
        public long insertPoint(ref DBPoint point)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO Points (ID, XCoordinate, YCoordinate, fkOutlineID, OrderID) " +
                        "VALUES (NULL, @XCoordinate, @YCoordinate, @fkOutlineID, @OrderID);";
                    cmd.Parameters.AddWithValue("@XCoordinate", point.xcoordinate);
                    cmd.Parameters.AddWithValue("@YCoordinate", point.ycoordinate);
                    cmd.Parameters.AddWithValue("@fkOutlineID", point.fkoutlineid);
                    cmd.Parameters.AddWithValue("@OrderID", point.orderid);

                    cmd.ExecuteNonQuery();

                    point.id = conn.LastInsertRowId;

                    conn.Close();

                    return point.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBInfo into DBInfo table
        //
        public void insertDBInfo(DBInfo dbinfo)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO DBInfo (Key, Value) " +
                        "VALUES (@Key, @Value);";
                    cmd.Parameters.AddWithValue("@Key", dbinfo.key);
                    cmd.Parameters.AddWithValue("@Value", dbinfo.value);

                    cmd.ExecuteNonQuery();

                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBOutline into Outlines table
        //
        public long insertOutline(ref DBOutline outline)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO Outlines (ID, TipPosition, BeginLE, EndLE, NotchPosition, EndTE, fkIndividualID) " +
                        "VALUES (NULL, @TipPosition, @BeginLE, @EndLE, @NotchPosition, @EndTE, @fkIndividualID);";
                    cmd.Parameters.AddWithValue("@TipPosition", outline.tipposition);
                    cmd.Parameters.AddWithValue("@BeginLE", outline.beginle);
                    cmd.Parameters.AddWithValue("@EndLE", outline.endle);
                    cmd.Parameters.AddWithValue("@NotchPosition", outline.notchposition);
                    cmd.Parameters.AddWithValue("@EndTE", outline.endte);
                    cmd.Parameters.AddWithValue("@fkIndividualID", outline.fkindividualid);

                    cmd.ExecuteNonQuery();

                    outline.id = conn.LastInsertRowId;

                    conn.Close();

                    return outline.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBImage into Images table
        //
        public long insertImage(ref DBImage image)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

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

                    conn.Close();

                    return image.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBImageModification into ImageModifications table
        //
        public long insertImageModification(ref DBImageModification imagemod)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

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

                    conn.Close();

                    return imagemod.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts DBThumbnail into Thumbnails table
        //
        public long insertThumbnail(ref DBThumbnail thumbnail)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "INSERT INTO Thumbnails (ID, Rows, Pixmap, fkImageID) " +
                        "VALUES (NULL, @Rows, @Pixmap, @fkImageID);";
                    cmd.Parameters.AddWithValue("@Rows", thumbnail.rows);
                    cmd.Parameters.AddWithValue("@Pixmap", thumbnail.pixmap);
                    cmd.Parameters.AddWithValue("@fkImageID", thumbnail.fkimageid);

                    cmd.ExecuteNonQuery();

                    thumbnail.id = conn.LastInsertRowId;

                    conn.Close();

                    return thumbnail.id;
                }
            }
        }

        // *****************************************************************************
        //
        // Inserts list of DBPoint's into Points table
        //
        public void insertPoints(List<DBPoint> points)
        {
            foreach (var p in points)
            {
                var pointCopy = p;
                insertPoint(ref pointCopy);
            }
        }

        // *****************************************************************************
        //
        // Inserts list of DBImageModification's into ImageModifications table
        //
        public void insertImageModifications(List<DBImageModification> imagemods)
        {
            foreach (var i in imagemods)
            {
                var modCopy = i;
                insertImageModification(ref modCopy);
            }
        }

        // *****************************************************************************
        //
        // Updates outline in Outlines table  
        //
        public void updateOutline(DBOutline outline)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "UPDATE Outlines SET " +
                        "TipPosition = @TipPosition, " +
                        "BeginLE = @BeginLE, " +
                        "EndLE = @EndLE, " +
                        "NotchPosition = @NotchPosition, " +
                        "fkIndividualID = @fkIndividualID " +
                        "WHERE ID = @ID";

                    cmd.Parameters.AddWithValue("@TipPosition", outline.tipposition);
                    cmd.Parameters.AddWithValue("@BeginLE", outline.beginle);
                    cmd.Parameters.AddWithValue("@EndLE", outline.endle);
                    cmd.Parameters.AddWithValue("@NotchPosition", outline.notchposition);
                    cmd.Parameters.AddWithValue("@fkIndividualID", outline.fkindividualid);
                    cmd.Parameters.AddWithValue("@ID", outline.id);

                    cmd.ExecuteNonQuery();

                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Updates row in DamageCategories table using given DBDamageCategory struct.
        // Uses ID field for identifying row.
        //
        public void updateDamageCategory(DBDamageCategory damagecategory)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

                    cmd.CommandText = "UPDATE DamageCategories SET " +
                        "Name = @Name, " +
                        "OrderID = @OrderID " +
                        "WHERE ID = @ID";

                    cmd.Parameters.AddWithValue("@Name", damagecategory.name);
                    cmd.Parameters.AddWithValue("@OrderID", damagecategory.orderid);
                    cmd.Parameters.AddWithValue("@ID", damagecategory.id);

                    cmd.ExecuteNonQuery();

                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Updates row in Individuals table using given DBIndividual struct.  Uses ID
        // field for identifying row.
        //
        public void updateIndividual(DBIndividual individual)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

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
                    conn.Close();
                }
            }
        }
        // *****************************************************************************
        //
        // Updates row in Images table using given DBImage struct.  Uses ID
        // field for identifying row.
        //
        public void updateImage(DBImage image)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();

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
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Updates row in ImageModifications table using given DBImageModification
        // struct.  Uses ID field for identifying row.
        //
        public void updateImageModification(DBImageModification imagemod)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
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
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Updates row in Thumbnails table using given DBThumbnail
        // struct.  Uses ID field for identifying row.
        //
        public void updateThumbnail(DBThumbnail thumbnail)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
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
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Updates row in DBInfo table using given DBInfo
        // struct.  Uses ID field for identifying row.
        //
        public void updateDBInfo(DBInfo dbinfo)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
                    cmd.CommandText = "UPDATE DBInfo SET " +
                        "Value = @Value " +
                        "WHERE Key = @Key";

                    cmd.Parameters.AddWithValue("@Value", dbinfo.value);
                    cmd.Parameters.AddWithValue("@Key", dbinfo.key);

                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Deletes set of points from Points table using fkOutlineID  
        //
        public void deletePoints(long fkOutlineID)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
                    cmd.CommandText = "DELETE FROM Points WHERE fkOutlineID = @ID";
                    cmd.Parameters.AddWithValue("@ID", fkOutlineID);

                    cmd.ExecuteNonQuery();

                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete outline from Outlines table using fkIndividualID  
        //
        public void deleteOutlineByFkIndividualID(long fkIndividualID)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
                    cmd.CommandText = "DELETE FROM Outlines WHERE fkIndividualID = @ID";
                    cmd.Parameters.AddWithValue("@ID", fkIndividualID);

                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete outline from Outlines table using id  
        //
        public void deleteOutlineByID(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    conn.Open();
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
        public void deleteIndividual(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM Individuals WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete damagecategory from DamageCategories table using id  
        //
        public void deleteDamageCategory(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM DamageCategories WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete image from Images table using id  
        //
        public void deleteImage(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM Images WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete imagemod from ImageModifications table using id  
        //
        public void deleteImageModification(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM ImageModifications WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Delete thumbnail from Thumbnails table using id  
        //
        public void deleteThumbnail(int id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM Thumbnails WHERE ID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        // *****************************************************************************
        //
        // Returns complete DatabaseFin<ColorImage>. mDataPos field will be used to map to id in 
        // db for individuals
        //
        public DatabaseFin getFin(long id)
        {
            DBIndividual individual;
            DBImage image;
            DBOutline outline;
            DBThumbnail thumbnail;
            DBDamageCategory damagecategory;
            Outline finOutline;
            FloatContour fc = new FloatContour();
            DatabaseFin fin;

            individual = selectIndividualByID(id);
            damagecategory = selectDamageCategoryByID(individual.fkdamagecategoryid);
            image = selectImageByFkIndividualID(id);
            outline = selectOutlineByFkIndividualID(id);
            thumbnail = selectThumbnailByFkImageID(image.id);
            List<DBPoint> points = selectPointsByFkOutlineID(outline.id);

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

            // Based on thumbnail size in DatabaseFin<ColorImage>
            int pixmapCols = thumbnail.pixmap.Length / thumbnail.rows;
            char[,] pixmap = new char[pixmapCols, thumbnail.rows];

            // Could be done faster with array.copy
            for (int i = 0; i < thumbnail.rows; i++)
            {
                for (int j = 0; j < pixmapCols; j++)
                {
                    pixmap[j, i] = thumbnail.pixmap[j + i * pixmapCols];
                }
            }

            fin = new DatabaseFin(image.imagefilename,
                finOutline,
                individual.idcode,
                individual.name,
                image.dateofsighting,
                image.rollandframe,
                image.locationcode,
                damagecategory.name,
                image.shortdescription,
                individual.id, // mDataPos field will be used to map to id in db for individuals
                pixmap,
                thumbnail.rows);

            return fin;
        }

        // *****************************************************************************
        //
        // Returns all fins from database.
        //
        public override List<DatabaseFin> GetAllFins()
        {
            List<DatabaseFin> fins = new List<DatabaseFin>();

            List<DBIndividual> individuals = selectAllIndividuals();

            if (individuals == null)
                return fins;

            foreach (var ind in individuals)
            {
                fins.Add(getFin(ind.id));
            }

            return fins;
        }

        // *****************************************************************************
        //
        // Delete thumbnail from Thumbnails table using fkImageID  
        //
        public void deleteThumbnailByFkImageID(long id)
        {
            using (var conn = new SQLiteConnection(_connectionString))
            {
                using (var cmd = new SQLiteCommand(conn))
                {
                    cmd.CommandText = "DELETE FROM Thumbnails WHERE fkImageID = @ID";
                    cmd.Parameters.AddWithValue("@ID", id);

                    conn.Open();
                    cmd.ExecuteNonQuery();
                    conn.Close();
                }
            }
        }

        public override long Add(DatabaseFin fin)
        {
            DBDamageCategory dmgCat;
            Outline finOutline;
            FloatContour fc;
            int i, numPoints;

            //***054 - assume that the image filename contains path
            // information which must be stripped BEFORE saving fin
            fin.mImageFilename = Path.GetFileName(fin.mImageFilename);

            // TODO
            //beginTransaction();

            dmgCat = selectDamageCategoryByName(fin.DamageCategory);

            if (dmgCat.id == -1)
                dmgCat = selectDamageCategoryByName("NONE");

            DBIndividual individual = new DBIndividual();
            individual.idcode = fin.IDCode;
            individual.name = fin.Name;
            individual.fkdamagecategoryid = dmgCat.id;
            insertIndividual(ref individual);

            finOutline = fin.mFinOutline;

            DBOutline outline = new DBOutline();
            outline.beginle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin);
            outline.endle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeEnd);
            outline.notchposition = finOutline.GetFeaturePoint(FeaturePointType.Notch);
            outline.tipposition = finOutline.GetFeaturePoint(FeaturePointType.Tip);
            outline.endte = finOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);
            outline.fkindividualid = individual.id;
            insertOutline(ref outline);

            List<DBPoint> points = new List<DBPoint>();
            numPoints = finOutline.Length;
            fc = finOutline.ChainPoints;
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
            insertPoints(points);

            DBImage image = new DBImage();
            image.dateofsighting = fin.DateOfSighting;
            image.imagefilename = fin.mImageFilename;
            image.locationcode = fin.LocationCode;
            image.rollandframe = fin.RollAndFrame;
            image.shortdescription = fin.ShortDescription;
            image.fkindividualid = individual.id;
            insertImage(ref image);

            DBThumbnail thumbnail = new DBThumbnail();
            thumbnail.rows = fin.ThumbnailRows;
            thumbnail.pixmap = new string(fin.ThumbnailPixmap.Cast<char>().ToArray()); ;
            thumbnail.fkimageid = image.id;
            insertThumbnail(ref thumbnail);

            //TODO
            //commitTransaction();

            addFinToLists(individual.id, individual.name, individual.idcode, image.dateofsighting,
                image.rollandframe, image.locationcode, dmgCat.name, image.shortdescription);

            sortLists();

            return individual.id; // mDataPos field will be used to map to id in db for individuals
        }

        // *****************************************************************************
        //
        // Updates DatabaseFin<ColorImage>
        //
        public void update(DatabaseFin fin)
        {
            DBImage image;
            DBOutline outline;
            DBThumbnail thumbnail;
            DBDamageCategory dmgCat;
            Outline finOutline;
            FloatContour fc;
            int i, numPoints;

            dmgCat = selectDamageCategoryByName(fin.DamageCategory);

            DBIndividual individual = new DBIndividual();
            individual.id = fin.DataPos; // mapping Individuals id to mDataPos
            individual.idcode = fin.IDCode;
            individual.name = fin.Name;
            individual.fkdamagecategoryid = dmgCat.id;
            updateIndividual(individual);

            finOutline = fin.mFinOutline;
            // we do this as we don't know what the outline id is
            outline = selectOutlineByFkIndividualID(individual.id);
            outline.beginle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeBegin);
            outline.endle = finOutline.GetFeaturePoint(FeaturePointType.LeadingEdgeEnd);
            outline.notchposition = finOutline.GetFeaturePoint(FeaturePointType.Notch);
            outline.tipposition = finOutline.GetFeaturePoint(FeaturePointType.Tip);
            outline.endte = finOutline.GetFeaturePoint(FeaturePointType.PointOfInflection);
            outline.fkindividualid = individual.id;
            updateOutline(outline);

            List<DBPoint> points = new List<DBPoint>();
            numPoints = finOutline.Length;
            fc = finOutline.ChainPoints;
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
            deletePoints(outline.id);
            insertPoints(points);

            // query db as we don't know the image id
            image = selectImageByFkIndividualID(individual.id);
            image.dateofsighting = fin.DateOfSighting;
            image.imagefilename = fin.mImageFilename;
            image.locationcode = fin.LocationCode;
            image.rollandframe = fin.RollAndFrame;
            image.shortdescription = fin.ShortDescription;
            image.fkindividualid = individual.id;
            updateImage(image);

            // query db as we don't know the thumbnail id
            thumbnail = selectThumbnailByFkImageID(image.id);
            thumbnail.rows = fin.ThumbnailRows;
            thumbnail.pixmap = new string(fin.ThumbnailPixmap.Cast<char>().ToArray());

            updateThumbnail(thumbnail);

            // loadLists(); // reload and re-sort lists.

            deleteFinFromLists(individual.id);
            addFinToLists(individual.id, individual.name, individual.idcode, image.dateofsighting,
                image.rollandframe, image.locationcode, dmgCat.name, image.shortdescription);

            sortLists();
        }


        // *****************************************************************************
        //
        // Delete fin from database
        //
        public override void Delete(DatabaseFin fin)
        {

            DBOutline outline;
            DBImage image;
            long id;

            // mDataPos field will be used to map to id in db for individuals
            id = fin.DataPos;

            //TODO
            //beginTransaction();

            outline = selectOutlineByFkIndividualID(id);
            image = selectImageByFkIndividualID(id);

            deletePoints(outline.id);
            deleteOutlineByFkIndividualID(id);
            deleteThumbnailByFkImageID(image.id);
            deleteImage(image.id);
            deleteIndividual(id);

            //TODO
            //commitTransaction();

            // TODO
            //deleteFinFromLists(id);
        }

        public void addFinToLists(DatabaseFin fin)
        {
            addFinToLists(fin.DataPos, fin.Name, fin.IDCode, fin.DateOfSighting,
                fin.RollAndFrame, fin.LocationCode, fin.DamageCategory,
                fin.ShortDescription);
        }

        //*******************************************************************
        //
        // Adds a fin to the sort lists. Does not resort the lists.
        //
        public void addFinToLists(long datapos, string name, string id, string date, string roll,
                                           string location, string damage, string description)
        {
            mNameList.Add((name ?? "NONE") + " " + datapos);
            mIDList.Add((id ?? "NONE") + " " + datapos);
            mDateList.Add((date ?? "NONE") + " " + datapos);
            mRollList.Add((roll ?? "NONE") + " " + datapos);
            mLocationList.Add((location ?? "NONE") + " " + datapos);
            mDamageList.Add((damage ?? "NONE") + " " + datapos);
            mDescriptionList.Add((description ?? "NONE") + " " + datapos);

            //***2.2 -- make room for HOLES, unused primary Keys
            // mAbsoluteOffset.push_back(datapos); // the way RJ did it

            // TODO
            //mAbsoluteOffset[datapos] = datapos;
        }

        private void deleteEntry(ref List<string> lst, long id)
        {
            for (var i = 0; i < lst.Count; i++)
            {
                if (listEntryToID(lst[i]) == id)
                {
                    lst.RemoveAt(i);
                    break;
                }
            }
        }

        public void deleteFinFromLists(long id)
        {
            deleteEntry(ref mNameList, id);
            deleteEntry(ref mIDList, id);
            deleteEntry(ref mDateList, id);
            deleteEntry(ref mRollList, id);
            deleteEntry(ref mLocationList, id);
            deleteEntry(ref mDamageList, id);
            deleteEntry(ref mDescriptionList, id);

            // TODO
            //***2.2 - replace all of above
            //if (id < mAbsoluteOffset.Count)
            //    mAbsoluteOffset[id] = -1;
        }

        //*******************************************************************
        //
        // Rebuilds the lists from the database and sorts them.
        //
        private void loadLists()
        {
            List<DatabaseFin> fins;

            if (mNameList == null)
                mNameList = new List<string>();
            else
                mNameList.Clear();

            if (mIDList == null)
                mIDList = new List<string>();
            else
                mIDList.Clear();

            if (mDateList == null)
                mDateList = new List<string>();
            else
                mDateList.Clear();

            if (mRollList == null)
                mRollList = new List<string>();
            else
                mRollList.Clear();

            if (mLocationList == null)
                mLocationList = new List<string>();
            else
                mLocationList.Clear();

            if (mDamageList == null)
                mDamageList = new List<string>();
            else
                mDamageList.Clear();

            if (mDescriptionList == null)
                mDescriptionList = new List<string>();
            else
                mDescriptionList.Clear();

            if (mAbsoluteOffset == null)
                mAbsoluteOffset = new List<long>();
            else
                mAbsoluteOffset.Clear();

            fins = GetAllFins();

            if (fins != null)
            {
                foreach (var fin in fins)
                {
                    addFinToLists(fin);
                }
            }

            sortLists();
        }

        public void sortLists()
        {
            if (mNameList != null)
                mNameList.Sort();
            if (mIDList != null)
                mIDList.Sort();
            if (mDateList != null)
                mDateList.Sort();
            if (mRollList != null)
                mRollList.Sort();
            if (mLocationList != null)
                mLocationList.Sort();
            if (mDamageList != null)
                mDamageList.Sort();
            if (mDescriptionList != null)
                mDescriptionList.Sort();
        }

        // *****************************************************************************
        //
        // Returns fin from database.  pos refers to position within one of the sort
        // lists.
        //
        public override DatabaseFin getItem(int pos)
        {
            long id;

            switch (mCurrentSort)
            {
                case DatabaseSortType.DB_SORT_NAME:
                    id = listEntryToID(mNameList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_ID:
                    id = listEntryToID(mIDList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_DATE:
                    id = listEntryToID(mDateList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_ROLL:
                    id = listEntryToID(mRollList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_LOCATION:
                    id = listEntryToID(mLocationList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_DAMAGE:
                    id = listEntryToID(mDamageList[pos]);
                    break;

                case DatabaseSortType.DB_SORT_DESCRIPTION:
                    id = listEntryToID(mDescriptionList[pos]);
                    break;

                default:
                    throw new NotImplementedException();
            }

            return getFin(id);
        }

        //*******************************************************************
        //
        // Looks up row id in AbsoluteOffset list and then uses getFin(int)
        // to retrieve that fin from the database.
        //
        public override DatabaseFin getItemAbsolute(int pos)
        {
            throw new NotImplementedException();
            //if (pos > this->mAbsoluteOffset.size())
            //    throw BoundsError();

            //if (mAbsoluteOffset[pos] == -1)
            //    return NULL;               // this is a HOLE, a previously deleted fin

            //DatabaseFin<ColorImage>* fin = getFin(this->mAbsoluteOffset[pos]);

            //return fin;
        }

        public override void CreateEmptyDatabase()
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

                    CREATE INDEX IF NOT EXISTS IF NOT EXISTS pts_order ON Points (OrderID);

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
                // At this point, the Database class already contains the catalog scheme 
                // specification.  It was set in the Database(...) constructor from 
                // a CatalogScheme passed into the SQLiteDatabase constructor - JHS

                for (int i = 0; i < mCatCategoryNames.Count; i++)
                {
                    DBDamageCategory cat = new DBDamageCategory
                    {
                        name = mCatCategoryNames[i],
                        orderid = i
                    };

                    insertDamageCategory(ref cat);
                }

                // TODO: enter code to populate DBInfo

                conn.Close();
            }
        }
    }
}