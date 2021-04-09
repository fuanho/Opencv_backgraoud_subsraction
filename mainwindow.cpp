#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

cv::Mat ToGray(cv::Mat frame)
{
    if(frame.channels() == 1) return frame.clone();
    cv::Mat process = cv::Mat(frame.rows, frame.cols, CV_8UC1);
    auto frame_data = frame.data;
    auto process_data = process.data;
    for(auto y = 0; y < frame.rows; y++)
    {
        for(auto x = 0; x < frame.cols; x++)
        {
            auto pixel = 0.114 * frame_data[0] + 0.578 * frame_data[1] + 0.299 * frame_data[2];
            process_data[0] = (uchar)pixel;
            frame_data += 3;
            process_data += 1;
        }
    }

    return process.clone();
}

cv::Mat background_subsraction_gray(cv::Mat first_frame, cv::Mat others_frame)
{
    cv::Mat process = cv::Mat(first_frame.rows, first_frame.cols, others_frame.type());
    auto first_data = first_frame.data;
    auto others_data = others_frame.data;
    auto process_data = process.data;
    for (auto y = 0; y < first_frame.rows; y++)
    {
        for (auto x = 0; x < first_frame.cols; x++)
        {
            auto pixel = abs(others_data[0] - first_data[0]);
            if(pixel <= 40) pixel = 0;
            else pixel = 255;
            process_data[0] = (uchar)pixel;
            first_data += 1;
            others_data += 1;
            process_data += 1;
        }
    }
    return process.clone();
}

cv::Mat background_subsraction_color(cv::Mat first_frame, cv::Mat others_frame) {
    cv::Mat process = cv::Mat(first_frame.rows, first_frame.cols, others_frame.type());
    auto first_data = first_frame.data;
    auto others_data = others_frame.data;
    auto process_data = process.data;
    for (auto y = 0; y < first_frame.rows; y++)
    {
        for (auto x = 0; x < first_frame.cols; x++)
        {
            auto pixel_B = abs(others_data[0] - first_data[0]);
            auto pixel_G = abs(others_data[1] - first_data[1]);
            auto pixel_R = abs(others_data[2] - first_data[2]);
            if (pixel_B <= 20) pixel_B = 0;
            else pixel_B = others_data[0];
            if (pixel_G <= 20) pixel_G = 0;
            else pixel_G = others_data[1];
            if (pixel_R <= 20) pixel_R = 0;
            else pixel_R = others_data[2];
            process_data[0] = (uchar)pixel_B;
            process_data[1] = (uchar)pixel_G;
            process_data[2] = (uchar)pixel_R;
            first_data += 3;
            others_data += 3;
            process_data += 3;
        }
    }

    return process.clone();
}

QList<QList<int>> scanner(cv::Mat frame)
{
    cv::Mat process = frame.clone();
    auto process_data = process.data;

    QList<int> lines_y;
    QList<int> lines_x;
    lines_x.push_back(0);
    lines_y.push_back(0);
    int width = frame.cols;
    int height = frame.rows;
    bool check = false;
    bool is_break = false;
    int last_pixel = 0;
    int pixel = 0;
    for(auto y = 0; y<height; y++)
    {
        process_data += y * width;
        for(auto x = 0; x<width; x++)
        {
            pixel = process_data[0];
            if(pixel != last_pixel)
            {
                is_break = true;
                break;
            }
            process_data += 1;
        }
        if(is_break && !check)
        {
            lines_y.push_back(y);
            check = true;
        }
        else if(!is_break && check)
        {
            lines_y.push_back(y);
            check = false;
        }
        is_break = false;
        last_pixel = pixel;
        process_data = process.data;
    }
    process_data = process.data;
    check = false;
    is_break = false;
    last_pixel = 0;
    pixel = 0;
    for(auto x = 0; x<width; x++)
    {
        process_data += x;
        for(auto y = 0; y<height; y++)
        {
            pixel = process_data[0];
            if(pixel != last_pixel)
            {
                is_break = true;
                break;
            }
            process_data += width;
        }
        if(is_break && !check)
        {
            lines_x.push_back(x);
            check = true;
        }
        else if(!is_break && check)
        {
            lines_x.push_back(x);
            check = false;
        }
        is_break = false;
        last_pixel = pixel;
        process_data = process.data;
    }

    lines_x.push_back(frame.cols-1);
    lines_y.push_back(frame.rows-1);
    QList<QList<int>> result;
    result.push_back(lines_x);
    result.push_back(lines_y);
    return result;
}

cv::Mat bounding_box(cv::Mat frame, QList<QList<int>> list)
{
    if(list[0].size() == 2 || list[1].size() == 2) return frame;
    cv::Mat process = frame.clone();
    QList<int> lines_x = list[0];
    QList<int> lines_y = list[1];
    int cursor_x = 0;
    int cursor_y = 0;
    int end_cursor_x = lines_x[0];
    int end_cursor_y = lines_y[0];
    auto process_data = process.data;

    int frame_pieces = (lines_x.size()-1) * (lines_y.size()-1);
    for(auto count = 0; count < frame_pieces; count++)
    {
        Rectantle rec(end_cursor_y, cursor_y, end_cursor_x, cursor_x);
        process_data = process.data;
        process_data += cursor_x + cursor_y*process.cols;
        for(auto y = cursor_y; y < end_cursor_y; y++)
        {
            process_data = process.data;
            process_data += cursor_x + y*process.cols;
            for(auto x = cursor_x; x < end_cursor_x; x++)
            {
                int pixel = process_data[0];
                if(pixel != 0)
                {
                    if(y > rec.bottom) rec.bottom = y;
                    if(x > rec.right) rec.right = x;
                    if(y < rec.top) rec.top = y;
                    if(x < rec.left) rec.left = x;
                }
                process_data += 1;
            }
        }
        process_data = process.data;
        process_data += cursor_x + cursor_y*process.cols;
        for(auto y = cursor_y; y < end_cursor_y; y++)
        {
            process_data = process.data;
            process_data += cursor_x + y*process.cols;
            for(auto x = cursor_x; x < end_cursor_x; x++)
            {
                auto pixel = 255;
                if(y == rec.top && x >= rec.left && x <= rec.right) process_data[0] = (uchar)pixel;
                else if(y == rec.bottom && x >= rec.left && x <= rec.right) process_data[0] = (uchar)pixel;
                else if(x == rec.left && y >= rec.top && y <= rec.bottom) process_data[0] = (uchar)pixel;
                else if(x == rec.right && y >= rec.top && y <= rec.bottom) process_data[0] = (uchar)pixel;
                process_data += 1;
            }
        }
        int which_x = count % (lines_x.size()-1);
        cursor_x = lines_x[which_x];
        end_cursor_x = lines_x[which_x+1];
        if(which_x == lines_x.size() - 2)
        {
            int which_y = (count)/(lines_x.size()-1);
            cursor_y = lines_y[which_y];
            end_cursor_y = lines_y[which_y+1];
        }
    }
    return process.clone();
}

void MainWindow::on_open_file_clicked()
{
    cv::VideoCapture cap("D:\\video.avi");
    cv::Mat first_frame;
    cv::Mat last_frame;
    cap >> first_frame;
    first_frame = ToGray(first_frame);
    last_frame = first_frame.clone();
    while(1)
    {
        cv::Mat image;
        cap >> image;
        image = ToGray(image);
        if(image.empty()) break;
        cv::Mat image2 = background_subsraction_gray(first_frame, image.clone());
        QList<QList<int>> scan_result;
        scan_result = scanner(image2);
        image2 = bounding_box(image2, scan_result);
        scan_result.clear();
        cv::Mat image3 = background_subsraction_gray(last_frame, image.clone());
        scan_result = scanner(image3);
        image3 = bounding_box(image3, scan_result);
        last_frame = image.clone();

        //cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
        //cv::cvtColor(image2, image2, cv::COLOR_BGR2RGB);
        //cv::cvtColor(image3, image3, cv::COLOR_BGR2RGB);
        ui->label->setPixmap(
                    QPixmap::fromImage(
                        QImage(image.data,image.cols,image.rows,image.step,QImage::Format_Grayscale8)
                        )
                    );

        ui->label_2->setPixmap(
                    QPixmap::fromImage(
                        QImage(image2.data, image2.cols, image2.rows, image2.step, QImage::Format_Grayscale8)
                        )
                    );
        ui->label_3->setPixmap(
                    QPixmap::fromImage(
                        QImage(image3.data,image3.cols,image3.rows,image3.step,QImage::Format_Grayscale8)
                        )
                    );
        //cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        //cv::cvtColor(image2, image2, cv::COLOR_RGB2BGR);
        //cv::cvtColor(image3, image3, cv::COLOR_RGB2BGR);
        char c = (char)cvWaitKey(25);
        if (c == 27) break;
    }
    cap.release();
    cv::destroyAllWindows();
}

void MainWindow::on_play_button_clicked()
{

}
