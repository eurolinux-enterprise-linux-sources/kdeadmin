/***************************************************************************
    begin                : Sun Oct 3 1999
    copyright            : (C) 1999 by Peter Putzer
    email                : putzer@kde.org
 ***************************************************************************/

/*
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef KSV_SERVICE_H
#define KSV_SERVICE_H



/**
 * The long description for this class goes here
 *
 * @short This is the short description
 * @author Peter Putzer <putzer@kde.org>
 * @version 0.1
 */
class KSVService 
{
public:

  /**
   * Named constructor
   */
  static KSVService* newService (const QString &name, const QString &basedir);

  /**
   * Destructor
   */
  ~KSVService();
  
  /**
   * Is the service active in runlevel "level"?
   */
  bool isOn (int level) const;

  /**
   * Is the service configured for runlevel "level"?
   */
  bool isConfigured (int level) const;
  
  /**
   * Set the service on or off in runlevel "level"
   */
  int set( int level, bool on = true );

  /**
   * Returns the description of the service
   */
  QString description () const;

private:
  /**
   * Copy Constructor
   */
  KSVService(const KSVService&);

  /**
   * Default Constructor
   */
  KSVService();

protected:
  friend class KServiceGUI;
  friend class KServiceManagerWidget;

  /**
   * Constructor
   */
  KSVService (const QString &name, const QString &basedir);

  /**
   * Name of the service
   */
  QString name_;

  /**
   * Description of the service (i.e. what it does)
   */
  QString desc_;
  
  /**
   * Base dir for storing runlevel dirs
   */
  QString base_;

  int levels, kPriority, sPriority;
};

#endif // KSV_SERVICE_H
