
#include <opencv2/opencv.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h> //for click event

#include <iostream>

#include <chrono>
#include <csignal>
#include <thread>

// Global variable to check if the loop should continue
bool continueLoop = true;

// Signal handler to handle interrupts and exit the loop
void signalHandler(int signum) {
  if (signum == SIGINT) {
    std::cout << "Received SIGINT. Exiting loop." << std::endl;
    continueLoop = false;
  }
}

// Function to display a Mat
void displayImage(const cv::Mat &image, const std::string &windowName) {
  cv::namedWindow(windowName, cv::WINDOW_NORMAL);
  cv::imshow(windowName, image);
  cv::waitKey(0);
  cv::destroyWindow(windowName);
  return;
}
// Function to take a screenshot using X11
void takeScreenshot(const std::string &filename) {
  Display *display = XOpenDisplay(nullptr);
  Window root = DefaultRootWindow(display);

  XWindowAttributes attributes;
  XGetWindowAttributes(display, root, &attributes);

  XImage *img = XGetImage(display, root, 0, 0, attributes.width,
                          attributes.height, AllPlanes, ZPixmap);

  cv::Mat screenshot(attributes.height, attributes.width, CV_8UC4, img->data);
  displayImage(screenshot, filename);
  XFree(img);
  XCloseDisplay(display);

  cv::imwrite(filename, screenshot);
}

// Function to simulate a left mouse click at the specified position
void simulateLeftClick(Display *display, int x, int y) {
  // Move the mouse pointer to the specified coordinates
  //XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);

  XTestFakeButtonEvent(display, 1, True, CurrentTime);
  XTestFakeButtonEvent(display, 1, False, CurrentTime);
  XFlush(display);
}
// Function to simulate a left mouse click at the specified position
void simulateLeftClick(Display* display, Window root, int x, int y) {
    Window child;
    XTranslateCoordinates(display, root, root, x, y, &x, &y, &child);

    // Move the mouse pointer to the specified coordinates
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);

    // Simulate the left button press and release
    XTestFakeButtonEvent(display, 1, True, CurrentTime);
    XTestFakeButtonEvent(display, 1, False, CurrentTime);

    // Return the mouse pointer to its original position
    XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);

    // Flush the X server to ensure events are processed
    XFlush(display);
}

int main() {
  Display *display = XOpenDisplay(nullptr);
  Window root = DefaultRootWindow(display);

  XWindowAttributes attributes;
  XGetWindowAttributes(display, root, &attributes);

  signal(SIGINT, signalHandler);

  while (continueLoop) {
    // continueLoop = true;
    //  takeScreenshot("screenshot.png"); // reference image to see if it works

    XImage *img = XGetImage(display, root, 0, 0, attributes.width,
                            attributes.height, AllPlanes, ZPixmap);

    cv::Mat sceneImage(attributes.height, attributes.width, CV_8UC4, img->data);
    // displayImage(sceneImage,"ss");
    //    XFree(img);
    //    XCloseDisplay(display);
    //      // Take a screenshot

    //     // Load template and scene images
    cv::Mat templateImage = cv::imread("template.png", cv::IMREAD_UNCHANGED);

    // cv::Mat sceneImage = cv::imread("screenshot.png", cv::IMREAD_UNCHANGED);

    if (templateImage.empty() || sceneImage.empty()) {
      std::cerr << "Error loading images." << std::endl;
      return -1;
    }

    // Create a mask based on the alpha channel of the template
    cv::Mat mask = templateImage.clone();
    cv::cvtColor(mask, mask, cv::COLOR_BGRA2GRAY);
    cv::threshold(mask, mask, 1, 255, cv::THRESH_BINARY);

    // Perform template matching with transparency and mask
    cv::Mat result;
    cv::matchTemplate(sceneImage, templateImage, result, cv::TM_CCOEFF_NORMED,
                      mask);
    // displayImage(result, "result");
    //  Define a threshold for matching
    double threshold = 0.95;

    // Find matches above the threshold
    cv::threshold(result, result, threshold, 1.0, cv::THRESH_BINARY);

    // Find location of the match
    cv::Point matchLoc;
    cv::minMaxLoc(result, nullptr, nullptr, nullptr, &matchLoc);

    // Check if a match is found
    if (result.at<float>(matchLoc) == 1.0) {
      std::cout << "Template found at: (" << matchLoc.x << ", " << matchLoc.y
                << ")" << std::endl;
      // Draw a rectangle around the detected region
         cv::Rect roi(matchLoc.x, matchLoc.y, templateImage.cols,
                     templateImage.rows);
         cv::rectangle(sceneImage, roi, cv::Scalar(0, 255, 0), 2);

      // Calculate the center coordinates
      int centerX = matchLoc.x + templateImage.cols / 2;
      int centerY = matchLoc.y + templateImage.rows / 2;
      simulateLeftClick(display,root, centerX, centerY);
      std::cout << "Clicked at: (" << centerX << ", " << centerY << ")"
                << std::endl;
    
      std::this_thread::sleep_for(std::chrono::seconds(
          3)); // wait at least 3 seconds before checking again.
      // Draw a red dot at the center
         cv::circle(sceneImage, cv::Point(centerX, centerY), 5,
                    cv::Scalar(0, 0, 255), -1);

      //   // Display the image with the bounding box and red dot
         cv::imshow("Detected Region", sceneImage);
         cv::waitKey(0);
         cv::destroyWindow("Detected Region");
    }
    // else {
    //   std::cout << "Template not found." << std::endl;
    // }
    std::this_thread::sleep_for(
        std::chrono::seconds(1)); // wait one second before checking again.
  }
  return 0;
}
