// main.cpp
// the intentionally vacant lines is that the formatter doesnt rearrage the
// imports.
// always include opencv first.
#include <X11/X.h>
#include <opencv2/opencv.hpp>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h> //for click event

#include <leptonica/allheaders.h>
#include <system_error>
#include <tesseract/baseapi.h>

#include <boost/program_options.hpp>
#include <iostream>

#include <chrono>
#include <csignal>
#include <thread>

namespace po = boost::program_options;

// Global variable to check if the loop should continue
bool continueLoop = true;
bool firstTimeOnly = false;
bool debug_images = false; // shows the images on the path to comparison
bool debug_info = false;   // shows 'checkpoints' INFO: messages
bool proofImage = false;   // shows the proofImage used in screenshots ;D
int debug_image_duration_in_seconds = 6;
int binary_threshold = 155;
int maxValue = 255;

void parseCommandLine(int argc, char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "first-time-only,f", "set first time only flag")("debug-images,i",
                                                       "enable debug images")(
      "debug-info,d", "enable debug info")("proof-image,p",
                                           "enable proof image")(
      "debug-image-duration,w", po::value<int>(),
      "debug image duration in seconds")("binary-threshold,t", po::value<int>(),
                                         "binary threshold");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    exit(0);
  }

  if (vm.count("debug-images"))
    debug_images = true;
  if (vm.count("debug-info"))
    debug_info = true;
  if (vm.count("proof-image"))
    proofImage = true;
  if (vm.count("debug-image-duration"))
    debug_image_duration_in_seconds =
        std::max(0, vm["debug-image-duration"].as<int>());
  if (vm.count("binary-threshold"))
    binary_threshold = std::max(0, vm["binary-threshold"].as<int>());
  if (vm.count("first-time-only"))
    firstTimeOnly = true;
}

// Signal handler to handle interrupts and exit the loop
void signalHandler(int signum) {
  try {
    cv::destroyAllWindows(); // just in case;
  } catch (std::error_code e) {
  }
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
// void takeScreenshot(const std::string &filename) {
//   Display *display = XOpenDisplay(nullptr);
//   Window root = DefaultRootWindow(display);

//   XWindowAttributes attributes;
//   XGetWindowAttributes(display, root, &attributes);

//   XImage *img = XGetImage(display, root, 0, 0, attributes.width,
//                           attributes.height, AllPlanes, ZPixmap);

//   cv::Mat screenshot(attributes.height, attributes.width, CV_8UC4,
//   img->data); displayImage(screenshot, filename); XFree(img);
//   XCloseDisplay(display);
// THIS WRITE TO FILE IS DEFININTELY NOT DOING WHAT IS INTENDED:
//   cv::imwrite(filename, screenshot);
// }

// Function to simulate a left mouse click at the specified position
// void simulateLeftClick(Display *display, int x, int y) {
//   // Move the mouse pointer to the specified coordinates
//   // XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);

//   XTestFakeButtonEvent(display, 1, True, CurrentTime);
//   XTestFakeButtonEvent(display, 1, False, CurrentTime);
//   XFlush(display);
// }
// Function to simulate a left mouse click at the specified position
void simulateLeftClick(Display *display, Window root, int x, int y) {
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
// Function to perform OCR using Tesseract
std::string performOCR(const cv::Mat &roi) {
  tesseract::TessBaseAPI ocr;
  ocr.Init(NULL,
           "eng"); // Change "eng" to the appropriate language code if necessary
  ocr.SetImage((uchar *)roi.data, roi.cols, roi.rows, 3, roi.cols * 3);

  // Perform OCR and get the result
  char *text = ocr.GetUTF8Text();
  std::string result(text);
  delete[] text;

  return result;
}
// Function to perform OCR on a specific region
std::string performOCR(const cv::Mat &image, const cv::Rect &roi) {
  // Extract the region of interest
  cv::Mat roiImage = image(roi).clone();

  l_uint32 *imageData = reinterpret_cast<l_uint32 *>(roiImage.data);

  // Convert the OpenCV Mat to a Leptonica Pix
  Pix *pix = pixCreate(roiImage.size().width, roiImage.size().height, 8);
  pixSetData(pix, imageData);

  // Perform OCR using Tesseract
  tesseract::TessBaseAPI api;
  api.Init(NULL, "eng"); // Initialize Tesseract with English language
  api.SetImage(pix);

  // Get OCR result
  char *ocrResult = api.GetUTF8Text();
  // std::string result
  std::string result(ocrResult);

  // Clean up resources
  delete[] ocrResult;
  pixDestroy(&pix);

  return result;
}
int main(int argc, char *argv[]) {
  if (argc > 0) {
    parseCommandLine(argc, argv);

    // Use the options as needed in your program
    std::cout << "Debug Images: " << debug_images << std::endl;
    std::cout << "Debug Info: " << debug_info << std::endl;
    std::cout << "Proof Image: " << proofImage << std::endl;
    std::cout << "Debug Image Duration: " << debug_image_duration_in_seconds
              << std::endl;
    std::cout << "Binary Threshold: " << binary_threshold << std::endl;
    std::cout << "example: ./find_template -i -d -p -w 10 -t 200 -f"
              << std::endl;
  }
  Display *display = XOpenDisplay(nullptr);
  Window root = DefaultRootWindow(display);

  XWindowAttributes attributes;
  XGetWindowAttributes(display, root, &attributes);

  signal(SIGINT, signalHandler);

  cv::Mat tempImage = cv::imread("template.png", cv::IMREAD_UNCHANGED);
  cv::Mat tempGrayImage;
  cv::cvtColor(tempImage, tempGrayImage, cv::COLOR_BGR2GRAY);

  // Apply thresholding to convert to binary image
  // int thresholdValue = 128; // Adjust this threshold value as needed
  // Maximum pixel value after thresholding
  cv::Mat templateImage;
  cv::threshold(tempGrayImage, templateImage, binary_threshold, maxValue,
                cv::THRESH_BINARY);

  // cv::Mat alphaChannel;
  //   cv::extractChannel(tempImage, alphaChannel,
  //                      3); // Assuming alpha channel is at index 3

  //   // Threshold the alpha channel to create a binary mask
  //   cv::Mat mask;
  //   cv::threshold(alphaChannel, mask, 0, 255, cv::THRESH_BINARY);
  //    if (debug_images) {
  //   cv::imshow("mask", mask);
  //   cv::waitKey(0); // no wait key
  //   //    cv::destroyWindow("bin image");
  // }
  // CANDO: can delete tempGray image here
  // CHJECKPOINT PASSEED
  if (debug_images) {
    cv::imshow("template", templateImage);
    cv::waitKey(debug_image_duration_in_seconds); // no wait key
    //    cv::destroyWindow("bin image");
  }
  while (continueLoop) {
    // continueLoop = true;
    //  takeScreenshot("screenshot.png"); // reference image to see if it works
    std::this_thread::sleep_for(
        std::chrono::seconds(6)); /// DEBUG: DELAY_BEFORE_START
    XImage *img = XGetImage(display, root, 0, 0, attributes.width,
                            attributes.height, AllPlanes, ZPixmap);

    cv::Mat sceneImage(attributes.height, attributes.width, CV_8UC4, img->data);
    // Convert image to grayscale
    cv::Mat grayImage;
    cv::cvtColor(sceneImage, grayImage, cv::COLOR_BGR2GRAY);

    // Apply thresholding to convert to binary image
    // int thresholdValue = 128; // Adjust this threshold value as needed
    // int maxValue = 255;       // Maximum pixel value after thresholding
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, binary_threshold, maxValue,
                  cv::THRESH_BINARY);
    /// THIS CHECKPOINT IS PASSED WITH FLYING COLORS
    if (debug_images) {
      cv::imshow("bin image", binaryImage);
      cv::waitKey(debug_image_duration_in_seconds);
    }
    //      cv::destroyWindow("bin image");
    // displayImage(sceneImage,"ss");
    //    XFree(img);
    //    XCloseDisplay(display);
    //      // Take a screenshot

    //     // Load template and scene images

    // cv::Mat sceneImage = cv::imread("screenshot.png", cv::IMREAD_UNCHANGED);

    if (templateImage.empty() || sceneImage.empty()) {
      std::cerr << "Error loading images." << std::endl;
      return -1;
    }

    //     // Create a mask based on the alpha channel of the template
    // cv::Mat mask = tempImage.clone();
    // std::cout << "INFO: tempImage cloned" << std::endl;
    // cv::threshold(mask, mask, 1, 255, cv::THRESH_BINARY);
    // std::cout << "INFO: created mask" << std::endl;
    //     // Perform template matching with transparency and mask

    // Extract the alpha channel from the template image

    cv::Mat result;
    if (debug_info)
      std::cout << "INFO: cv::Mat result created" << std::endl;
    cv::matchTemplate(binaryImage, templateImage, result,
                      cv::TM_CCOEFF_NORMED /*,mask*/);
    if (debug_info)
      std::cout << "INFO: matched template" << std::endl;
    if (debug_images) {
      cv::imshow("match result", result);
      cv::waitKey(debug_image_duration_in_seconds);
    }
    // displayImage(result, "result");
    //  Define a threshold for matching
    double threshold = 0.75;

    // Find matches above the threshold
    cv::threshold(result, result, threshold, 1.0, cv::THRESH_BINARY);
    // std::cout << "INFO: applied threshold" << std::endl;

    // Find location of the match
    cv::Point matchLoc;

    cv::minMaxLoc(result, nullptr, nullptr, nullptr, &matchLoc);
    //  std::cout << "INFO: calculated minMaxLoc" << std::endl;

    // Check if a match is found
    if (result.at<float>(matchLoc) == 1.0) {
      // std::cout << "Template found at: (" << matchLoc.x << ", " << matchLoc.y
      //           << ")" << std::endl;
      cv::Rect roiRect(matchLoc.x, matchLoc.y, templateImage.cols,
                       templateImage.rows);
      int centerX = matchLoc.x + templateImage.cols / 2;
      int centerY = matchLoc.y + templateImage.rows / 2;

      cv::Mat roi = binaryImage(roiRect);
      if (debug_images || proofImage) {
        // Draw a rectangle around the detected region

        //  only need to draw rectangle if displaying debug

        cv::circle(sceneImage, cv::Point(centerX, centerY), 5,
                   cv::Scalar(0, 0, 255), -1);

        cv::rectangle(sceneImage, roiRect, cv::Scalar(0, 255, 0), 2);
        // cv::imshow("roi", roi);
        cv::imshow("here?", sceneImage);
        // cv::waitKey(debug_image_duration_in_seconds);
        if (firstTimeOnly) {
          std::cout << "INFO: firsttime only activated" << std::endl;

          continueLoop = false;
        }

        cv::waitKey(0);
      }
      // cv::destroyWindow("roi");
      //  std::cout << "INFO: BINARY ROI" << std::endl;
      //
      simulateLeftClick(display, root, centerX, centerY);
       std::this_thread::sleep_for(std::chrono::seconds(1));
      // std::string ocrResult = performOCR(roi);

      // Print the OCR result
      // if (debug_info) {
      // } // intentional for now: want to see if OCR has any resutls
      // std::cout << "DEBUG:   OCR Result: " << ocrResult << std::endl;
      continue; // skip comparing the ocr findings
      // Check if the OCR result contains specific text
      // if (ocrResult.find("skip") != std::string::npos) {
      //   // If the specific text is found, perform the click or other actions
      //   // Calculate the center coordinates
      //   simulateLeftClick(display, root, centerX, centerY);
      //   std::cout << "Clicked at: (" << centerX << ", " << centerY << ")"
      //             << std::endl;

      //   // std::this_thread::sleep_for(
      //   //     std::chrono::seconds(3)); // wait at least 3 seconds before
      //   //     checking
      //   //                               // again. Draw a red dot at the
      //   center cv::circle(sceneImage, cv::Point(centerX, centerY), 5,
      //              cv::Scalar(0, 0, 255), -1);

      //   // cv::imwrite("fail.png", sceneImage);
      //   //   // Display the image with the bounding box and red dot
      //   if (proofImage) {
      //     cv::imshow("Detected Region", sceneImage);
      //     cv::waitKey(8);
      //   }
      //   // cv::destroyWindow("Detected Region");
      //   std::this_thread::sleep_for(std::chrono::seconds(1));
      // }
    }
    // else {
    //   std::cout << "Template not found." << std::endl;
    // }
    XFree(img);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // wait one second before checking again.
  }

  XCloseDisplay(display);
  cv::destroyAllWindows();
  return 0;
}
