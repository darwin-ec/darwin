///////////////////////////////////////////////////////////////////////////
//                                                                       //
//	DARWIN Software                                                      //
//                                                                       //
// Digital Analysis and Recognition of Whale Image on a Network          //
// Digital Analysis to Recognize Whale Images on a Network               //
// Digital Archive of Recognized Whale Images on a Network               //
//                                                                       //
// Software designed and coded as a collaborative effort ...             //
//                                                                       //
// ... by  The DARWIN Research Group                                     //
//                                                                       //
//	Kelly R. Debure, PhD (GTK+, X Window System version)                 //
//	John H. Stewman, PhD (all versions)                                  //
//	Kristen McCoy (GTK+ Version)                                         //
//	Henry Burroughs (GTK+ Version)                                       //
//	Zach Roberts (X Window System version)                               //
//	Daniel J. Wilkin (X Window System version)                           //
//	Mark C. Allen (Microsoft Win 3.1 version, Expert System version)     //
//	Rachel Stanley (Expert System version)                               //
//	Andrew Cameron (protype database)                                    //
//	Adam Russell (major Object Oriented rewrite & Gtk GUI)               //
//  Scott Hale (autotrace, finz file support, misc. gui/HCI ...)         //
//  R J Nowling (SQLite database, finz file support)                     //
//                                                                       //
// ... at  Eckerd College                                                //
//	       St.Petersburg, FL                                             //
//                                                                       //
// xdarwin (DARWIN for the X Window System)                              //
//                                                                       //
// ... begun on July 27, 1994 (JHS)                                      //
// ... and updated on July 28, 1994 (JHS)                                //
// ... and updated starting June 12, 1995 (DW)                           //
// ... then maintained by Daniel J. Wilkin and Zach Roberts              //
// ... major rewrite using GTK+ 2000-2002 (Adam Russell)                 //
// ... currently maintained by JHS & KRD                                 //
//                                                                       //
// Supported by grants from the National Science Foundation              //
//                                                                       //
// Species Domain - Common Bottlenose Dolphin - Tursiops truncatus       //
//                                                                       //
// Copyright (C) 2001                                                    //
///////////////////////////////////////////////////////////////////////////

using Darwin.Database;
using Darwin.Wpf.ViewModel;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using Darwin.Model;

namespace Darwin.Wpf
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private void Application_Startup(object sender, StartupEventArgs e)
        {
            if (e.Args != null && e.Args.Length > 0)
            {
                if (!string.IsNullOrEmpty(e.Args[0]) && e.Args[0].ToLower().EndsWith(".finz"))
                {
                    var fin = CatalogSupport.OpenFinz(e.Args[0]);

                    // TODO: Better error messages?
                    if (fin == null)
                    {
                        var result = MessageBox.Show("Problem opening finz file.");
                        System.Windows.Application.Current.Shutdown();
                    }
                    else
                    {
                        // TODO: Hack for HiDPI
                        fin.ModifiedFinImage.SetResolution(96, 96);

                        // TODO: Move this logic into the constructor?
                        var vm = new TraceWindowViewModel(
                            fin.mFinImage ?? fin.ModifiedFinImage,
                            new Contour(fin.FinOutline.ChainPoints, fin.Scale),
                            fin.FinOutline,
                            true,
                            true);

                        TraceWindow traceWindow = new TraceWindow(vm);
                        traceWindow.Show();
                    }
                }
                else
                {
                    MessageBox.Show("Unknown commandline arguments.");
                    System.Windows.Application.Current.Shutdown();
                }
            }
            else
            {
                StartupUri = new Uri("/Darwin.Wpf;component/MainWindow.xaml", UriKind.Relative);
            }
        }
    }
}
