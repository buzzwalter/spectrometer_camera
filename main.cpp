#include "mainwindow.h"

#include <typeinfo>
#include <QApplication>
#include <iostream>
#include "MT_USBCamera_SDK.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <QtCharts>
#include <math.h>
#include <cmath>
#include <fstream>
#include <ctime>
#include <sstream>
//#include <filesystem>
#include <cstdlib>
using namespace std;

// fit_Gaussian is a function that fits a gaussian to a set of x and y pairs via least squares
//   int N number of points
//   double x[N] independent samples
//   double y[N] dependent samples
//   double a[3] container for coefficients read out in order x^0, x^1, ...
void fit_Gaussian(int N, double* x, double* y_precursor, double* a){
    int i, j, k, n;
    double y[N];
    for (int log_index = 0; log_index < N; log_index++){
        y[log_index] = std::log(y_precursor[log_index]); // calculate log to convert problem of gaussian fit to parabola fit
    }

    n = 2;  // fitting a second degree polynomial
    double X[2*n+1];                        //Array that will store the values of sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
    for (i=0;i<2*n+1;i++)
    {
        X[i]=0;
        for (j=0;j<N;j++)
            X[i]=X[i]+pow(x[j],i);        //consecutive positions of the array will store N,sigma(xi),sigma(xi^2),sigma(xi^3)....sigma(xi^2n)
    }
    double B[n+1][n+2];            //B is the Normal matrix(augmented) that will store the equations
    for (i=0;i<=n;i++)
        for (j=0;j<=n;j++)
            B[i][j]=X[i+j];            //Build the Normal matrix by storing the corresponding coefficients at the right positions except the last column of the matrix
    double Y[n+1];                    //Array to store the values of sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    for (i=0;i<n+1;i++)
    {
        Y[i]=0;
        for (j=0;j<N;j++)
            Y[i]=Y[i]+pow(x[j],i)*y[j];        //consecutive positions will store sigma(yi),sigma(xi*yi),sigma(xi^2*yi)...sigma(xi^n*yi)
    }
    for (i=0;i<=n;i++)
        B[i][n+1]=Y[i];                //load the values of Y as the last column of B(Normal Matrix but augmented)
    n=n+1;                //n is made n+1 because the Gaussian Elimination part below was for n equations, but here n is the degree of polynomial and for n degree we get n+1 equations
//    cout<<"\nThe Normal(Augmented Matrix) is as follows:\n";
//    for (i=0;i<n;i++)            //print the Normal-augmented matrix
//    {
//        for (j=0;j<=n;j++)
//            cout<<B[i][j]<<setw(16);
//        cout<<"\n";
//    }
    for (i=0;i<n;i++)                    //From now Gaussian Elimination starts(can be ignored) to solve the set of linear equations (Pivotisation)
        for (k=i+1;k<n;k++)
            if (B[i][i]<B[k][i])
                for (j=0;j<=n;j++)
                {
                    double temp=B[i][j];
                    B[i][j]=B[k][j];
                    B[k][j]=temp;
                }

    for (i=0;i<n-1;i++)            //loop to perform the gauss elimination
        for (k=i+1;k<n;k++)
        {
            double t=B[k][i]/B[i][i];
            for (j=0;j<=n;j++)
                B[k][j]=B[k][j]-t*B[i][j];    //make the elements below the pivot elements equal to zero or elimnate the variables
        }
    for (i=n-1;i>=0;i--)                //back-substitution
    {                        //x is an array whose values correspond to the values of x,y,z..
        a[i]=B[i][n];                //make the variable to be calculated equal to the rhs of the last equation
        for (j=0;j<n;j++)
            if (j!=i)            //then subtract all the lhs values except the coefficient of the variable whose value                                   is being calculated
                a[i]=a[i]-B[i][j]*a[j];
        a[i]=a[i]/B[i][i];            //now finally divide the rhs by the coefficient of the variable to be calculated
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    HWND handle;
    handle = HWND(w.winId());
    unsigned char* tempBuffer = new unsigned char[327680]();
    int TotalFrames = 100;
    int FrameType = 1;
    int deviceID = 0; // first and only mightex camera hooked up
    MTUSB_InitDevice(); // initialize device
    DEV_HANDLE device_handle = MTUSB_OpenDevice(deviceID); // Open said device and return its handle
    MTUSB_StartCameraEngine(handle, device_handle);
    MTUSB_StartFrameGrab(device_handle, TotalFrames);
    MTUSB_GetCurrentFrame(device_handle, FrameType, tempBuffer);



//    for (int i = 0; i < 10; i++){
//        printf("This is the value of the %dth element: %d \n", i, tempBuffer[i]);
//    }
//    PImageCtl myImgPtr = new TImageControl();
//    MTUSB_GetFrameSetting(device_handle, myImgPtr);
//    printf("This is the last element: %d \n", tempBuffer[327679]);
//    printf("This is the resolution encoding: %d \n", myImgPtr->Resolution); // Look in Mightex manual to see encoding scheme
//    printf("This is the bin setting: %d \n", myImgPtr->BinMode);


    QLineSeries *series = new QLineSeries();
    double fit_profile[512]; // to hold converted values for fitting
    double fit_indicies[512]; // to hold converted values for fitting (will adjust to calibrated values)
//    int max = tempBuffer[start_index];
    int start_index = (640/2-1)*512;
    for (int i = 0; i < 512; i++){
//        if (tempBuffer[start_index+i] > max) {
//            max = tempBuffer[start_index+i];
//            }
        series->append(i,tempBuffer[start_index+i]);
        fit_profile[i] = static_cast<double>(tempBuffer[start_index+i]);
        fit_indicies[i] = static_cast<double>(i);
        //std::cout << fit_profile[i] << std::endl;
        //printf("This is %dth pixel", static_cast<double>(3));
    }

    // fit gaussian profile and store coeffs in a
    double coeffs[3];
    fit_Gaussian(512,fit_indicies,fit_profile,coeffs);

    QChart *chart = new QChart();

    double max = -coeffs[1]/(2*coeffs[2]);
    double bandwidth = 2*std::sqrt(1/coeffs[2]);
    std::cout << max << std::endl;
    std::cout << bandwidth << std::endl;

    char graphics_text_string[50];
    std::sprintf(graphics_text_string,"The max is %0.3f and the bandwidth is %0.3f", max, bandwidth);
    QGraphicsTextItem textItem = QGraphicsTextItem(graphics_text_string, chart);
    textItem.setPos(10,10);


    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();

    QFont font;
    font.setPixelSize(18);
    chart->setTitleFont(font);
    chart->setTitleBrush(QBrush(Qt::black));
    chart->setTitle("Central Cut");

    QPen pen(QRgb(0x000000));
    pen.setWidth(5);
    series->setPen(pen);

    chart->setAnimationOptions(QChart::AllAnimations);

    QString* index_str = new QString();
    QCategoryAxis *axisX = new QCategoryAxis();
    for (int i =  0; i<512; i++){
        *index_str = QString::fromStdString(std::to_string(i));
        axisX->append(*index_str,i);
    }
    delete index_str;
    chart->setAxisX(axisX, series);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);



    QMainWindow window;

    window.setCentralWidget(chartView);
    window.resize(420,300);
    window.show();

    // Write data to txt file in working directory
    auto now = std::chrono::system_clock::now(); // Gets time object
    std::time_t time = std::chrono::system_clock::to_time_t(now); // Converts it to lower precision

    // Converts it to string in the "YYYY-MM-DD_HH-MM-SS" format

// for newer version of c++
//    std::stringstream ss;
//    ss << std::put_time(std::localtime(&time),"%Y-%m-%d_%H-&M-%S");
//    std::string date_string = ss.str();
// for older versions of c++
    char buffer[80];
    std::strftime(buffer,sizeof(buffer),"%Y-%m-%d_%H-%M-%S",std::localtime(&time));
    std::string date_string(buffer);

    // Get to current directory
    SetCurrentDirectory("C:/Users/liangyin/Documents/spectrometer_camera");

    // Create and open file
    std::string file_name = date_string + ".txt";
    std::ofstream output_file(file_name);

    // Check if file can be opened and write
    if (output_file.is_open()) {
        std::cout << "File opened" << std::endl;
        for (int i = 0; i < sizeof(fit_profile); i++){
            output_file << fit_indicies << "\t" <<  fit_profile <<std::endl; // Tab delimited two column file
        }
        output_file.close(); // Make sure to close file

// Get current directory
// newer version of c++
//        std::string directory = std::filesystem::current_path().string();
// older versions

        char* current_path = std::getenv("PWD") ? std::getenv("PWD") : std::getenv("USERPROFILE");
        if (current_path != nullptr){
            std::string directory(current_path);
            std::cout << "File written to " << directory << std::endl;
        }
    }
    else {
        std::cout << "Error opening file" << std::endl;
    }

    return a.exec();
}
