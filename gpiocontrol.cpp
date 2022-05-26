#include "gpiocontrol.h"
#include <QTextStream>
#include <QFile>

//! Port directory and file constants
QString GpioControl::baseGPIOPath_ = "/sys/class/gpio/";
QString GpioControl::gpioExport_ = "export";
QString GpioControl::gpioUnexport_ = "unexport";
QString GpioControl::gpioDirection_ = "direction";
QString GpioControl::gpioValue_ = "value";

GpioControl::GpioControl(QObject *parent) : QObject(parent)
{

}

bool GpioControl::writeToGPIOFile( const QString file, QString outputData )
{
  QFile exportFile( file );

  if (exportFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream exportStream( &exportFile );

    exportStream << outputData;

    exportFile.close();

    return true;
   }

  //qDebug() << "Error writing " << file;

  return false;
}

bool GpioControl::readFromGPIOFile( const QString file, QString &inputData )
{
  QFile inputFile( file );

  if ( inputFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream inputStream( &inputFile );

    inputStream >> inputData;

    inputFile.close();

    return true;
   }

  //qDebug() << "Error reading " << file;

  return false;
}

bool GpioControl::exportGPIO( int pinNumber )
{
  return writeToGPIOFile( baseGPIOPath_ + gpioExport_, QString::number(pinNumber) );
}

bool GpioControl::unexportGPIO( int pinNumber )
{
  return writeToGPIOFile( baseGPIOPath_ + gpioUnexport_, QString::number(pinNumber) );
}

bool GpioControl::setDirection( int pinNumber, SET_DIR read )
{
  bool result = false;
  QString path = baseGPIOPath_ + "gpio" + QString::number(pinNumber) + "/" + gpioDirection_;

  if ( read )
    result = writeToGPIOFile( path, "in" );
  else
    result = writeToGPIOFile( path, "out" );

  return result;
}

bool GpioControl::setValue(int pinNumber, GpioControl::SET_VAL value )
{
  bool result = false;
  QString path = baseGPIOPath_ + "gpio" + QString::number(pinNumber) + "/" + gpioValue_;

  if ( value )
    result = writeToGPIOFile( path, "1" );
  else
    result = writeToGPIOFile( path, "0" );

  return result;
}

bool GpioControl::readValue( int pinNumber, bool &value )
{
  bool result = false;
  QString data = "";
  QString path = baseGPIOPath_ + "gpio" + QString::number(pinNumber) + "/" + gpioValue_;

  result = readFromGPIOFile( path, data );

  if ( result )
    value = (data == "1");

  return result;
}
