
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "featurePic.hpp"
#include "featurePicHelp.hpp"
#include "acknowledgments.hpp"


double settings_version = 2.0;


featurePic::featurePic (int32_t *argc __attribute__ ((unused)), char **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;
  BFDATA_HEADER   bfd_header;
  BFDATA_RECORD   bfd_record;
  uint8_t         *image = NULL;
  int32_t         bfd_handle, record;
  double          heading;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/feature_pic.png"));


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("chartsPic main toolbar"));

  envin ();


  // Set the application font

  QApplication::setFont (font);


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  statusBar ()->setSizeGripEnabled (false);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the pic.

  picdef.transMode = Qt::SmoothTransformation;
  pic = new nvPic (this, &picdef);


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  vBox->addWidget (pic);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.png"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  tools->addSeparator ();
  tools->addSeparator ();


  QToolButton *bBrighten = new QToolButton (this);
  bBrighten->setIcon (QIcon (":/icons/brighten.png"));
  bBrighten->setToolTip (tr ("Brighten the picture"));
  connect (bBrighten, SIGNAL (clicked ()), this, SLOT (slotBrighten ()));
  tools->addWidget (bBrighten);


  QToolButton *bDarken = new QToolButton (this);
  bDarken->setIcon (QIcon (":/icons/darken.png"));
  bDarken->setToolTip (tr ("Darken the picture"));
  connect (bDarken, SIGNAL (clicked ()), this, SLOT (slotDarken ()));
  tools->addWidget (bDarken);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about chartsPic"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgments = new QAction (tr ("A&cknowledgments"), this);
  acknowledgments->setShortcut (tr ("Ctrl+c"));
  acknowledgments->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgments, SIGNAL (triggered ()), this, SLOT (slotAcknowledgments ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgments);
  helpMenu->addAction (aboutQtAct);


  int32_t option_index = 0;
  heading = 0.0;
  record = -1;
  char filename[512];


  while (NVTrue) 
    {
      static struct option long_options[] = {{"file", required_argument, 0, 0},
                                             {"record", required_argument, 0, 0},
                                             {"heading", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      char c = (char) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;


      QString skey;


      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              strcpy (filename, optarg);
              break;

            case 1:
              sscanf (optarg, "%d", &record);
              break;

            case 2:
              sscanf (optarg, "%lf", &heading);
              break;

            default:
              break;
            }
          break;
        }
    }


  show ();


  //  If we specified a record on the command line, this must be a BFD file so we'll read the image from the file.

  if (record >= 0)
    {
      if ((bfd_handle = binaryFeatureData_open_file (filename, &bfd_header, BFDATA_READONLY)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, "featurePic", tr ("Unable to open feature file.\nReason: %1").arg (msg));
          exit (-1);
        }

      if (binaryFeatureData_read_record (bfd_handle, record, &bfd_record) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, "featurePic", tr ("Unable to read the feature record.\nReason: %1").arg (msg));
          exit (-1);
        }

      image = (uint8_t *) malloc (bfd_record.image_size);

      if (image == NULL)
        {
          QString msg = QString (strerror (errno));
          QMessageBox::warning (this, "featurePic", tr ("Error allocating image memory.\nReason: %1").arg (msg));
          exit (-1);
        }

      if (binaryFeatureData_read_image (bfd_handle, record, image) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, "featurePic", tr ("Unable to read the image data.\nReason: %1").arg (msg));
          exit (-1);
        }


      //  Load the image

      if (!pic->OpenData (image, bfd_record.image_size))
        {
          QMessageBox::warning (this, "featurePic", tr ("There was an error opening the image from file %1").arg (QDir::toNativeSeparators (QString (filename))));
          exit (-1);
        }

      free (image);
    }
  else
    {
      //  Load the picture.

      if (!pic->OpenFile (filename))
        {
          QMessageBox::warning (this, "featurePic", tr ("There was an error opening or reading the file %1").arg (QDir::toNativeSeparators (QString (filename))));
          exit (-1);
        }
    }


  //  Rotate to "north up" if available

  pic->RotatePixmap (heading);

  pic->setCursor (Qt::ArrowCursor);

  pic->enableSignals ();
}



featurePic::~featurePic ()
{
}


void
featurePic::redrawPic ()
{
  pic->redrawPic ();
}



//  A bunch of slots.

void 
featurePic::slotQuit ()
{
  envout ();

  exit (0);
}



void 
featurePic::slotBrighten ()
{
  pic->brighten (10);
}



void 
featurePic::slotDarken ()
{
  pic->brighten (-10);
}



void
featurePic::about ()
{
  QMessageBox::about (this, VERSION, tr ("featurePic - Feature snippet file viewer.\n\nAuthor : Jan C. Depner (jan@pfmabe.software)"));
}


void
featurePic::slotAcknowledgments ()
{
  QMessageBox::about (this, VERSION, acknowledgmentsText);
}


void
featurePic::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}


//  Get the users defaults.

void
featurePic::envin ()
{
  double saved_version = 0.0;


  // Set defaults so the if keys don't exist the parameters are defined

  window_x = 0;
  window_y = 0;
  width = PIC_X_SIZE;
  height = PIC_Y_SIZE;
  font = QApplication::font ();


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/featurePic.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/featurePic.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("featurePic");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  width = settings.value (QString ("width"), width).toInt ();

  height = settings.value (QString ("height"), height).toInt ();

  window_x = settings.value (QString ("window x"), window_x).toInt ();

  window_y = settings.value (QString ("window y"), window_y).toInt ();


  this->restoreState (settings.value (QString ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();


  //  We need to get the font from the global settings since it will be used in (hopefully) all of the ABE map GUI applications.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  font.fromString (fontString);


  settings2.endGroup ();
}




//  Save the users defaults.

void
featurePic::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/featurePic.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/featurePic.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("featurePic");


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("width"), width);

  settings.setValue (QString ("height"), height);

  settings.setValue (QString ("window x"), window_x);

  settings.setValue (QString ("window y"), window_y);


  settings.setValue (QString ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
