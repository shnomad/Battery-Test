#ifndef GPIOCONTROL_H
#define GPIOCONTROL_H

#include <QObject>

class GpioControl : public QObject
{
    Q_OBJECT
public:
    explicit GpioControl(QObject *parent = nullptr);

    enum SET_DIR{
         SET_OUT=0,
         SET_IN
     };

     enum SET_VAL{
         SET_LOW=0,
         SET_HIGH
     };

     //! Export Port for using
     bool exportGPIO( int pinNumber );
     //! Unexport Port
     bool unexportGPIO( int pinNumber );
     //! Set the direction (read/write) of the specified port
     bool setDirection( int pinNumber, SET_DIR);
     //! Write to the specified port
     bool setValue( int pinNumber, SET_VAL);
     //! Read from the specified port
     bool readValue( int pinNumber, bool &value );

 signals:

 public slots:

private:
    static QString baseGPIOPath_;
    static QString gpioExport_;
    static QString gpioUnexport_;
    static QString gpioDirection_;
    static QString gpioValue_;

    //! Helper function for writing to the GPIO directory
    bool writeToGPIOFile(const QString file, QString outputData);
    //! Helper function for reading from the GPIO directory
    bool readFromGPIOFile(const QString file, QString &inputData);

};

#endif // GPIOCONTROL_H
