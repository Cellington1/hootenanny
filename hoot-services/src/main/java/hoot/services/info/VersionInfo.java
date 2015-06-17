/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2014 DigitalGlobe (http://www.digitalglobe.com/)
 */
package hoot.services.info;

import javax.xml.bind.annotation.XmlRootElement;

/**
 * Information about the hoot version
 */
@XmlRootElement
public class VersionInfo 
{
  private String name;
  public String getName() { return name; }
  public void setName(String name) { this.name = name; }
  
  private String version;
  public String getVersion() { return version; }
  public void setVersion(String version) { this.version = version; }
  
  private String builtBy;
  public String getBuiltBy() { return builtBy; }
  public void setBuiltBy(String builtBy) { this.builtBy = builtBy; }
	
  public VersionInfo()
  {
    
  }
  
  public String toString()
  {
    return "Name: " + name + "\nVersion: " + version + "\nBuilt By: " + builtBy;
  }
}
