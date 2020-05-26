/////////////////////////////////////////////////////////////////////
//   file: CatalogScheme.h
//
// author: J H Stewman
//
//   date: 7/18/2008
//
// new catalog scheme class - begin integrating with all code - JHS
//
/////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;

namespace Darwin.Database
{
    public class CatalogScheme
    {
		public string SchemeName { get; set; }
		public List<string> CategoryNames { get; set; }

		public CatalogScheme()
		{
			SchemeName = string.Empty;
			CategoryNames = new List<string>();
		}
	}
}
