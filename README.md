#skipAd

Simple implementation of openCV template matching to automatically click "Skip" as soon as the option pops up in youtube. 

<h2>What Technology is in this study?</h2>
<ul><li>OpenCV</li>
<li>Tesseract OCR - optical character recognition</li></ul>

<h2>Why not just use addblock?</h2>
apparently it violates the youtube user agreement. 

<h2>Why not write native browser plugin?</2>
Agreed that a native plugin can scan the site for an input with the text "Skip" and send a click event? 
<h6>Simply Put:</h6> 
I did it this way specifically to apply some OpenCV for preprocessing and template matching
I applied OCR to confirm the ROI (Region of Interest) contains the actual text "skip" 

<h2>How well does it work?</h2>
It is hard to tell: after some super fast reflexes in skipping adds, youtube removes them completely for some time and prevents me from testing many incidents in quick succession. 
<h6>What is my conclusion?</h6>
I conclude that the adds algorithm prevents you from being overburdened with too many adds in a short timeframe, specifically if you watch two adds and then video hop, you'll not be shown more adds on every new video: 

<h2>What would I do different next time?</h2>
I think a cool idea would be to open chrome and youtube in a seperate window on a virtual screen, and let it run the adds every two or so minutes. This will make other tabs in youtube less likely to show adds to the viewer. Although I believe it is possible I feel this will be more unethical than current solution of me just wanting to watch videos without having to to skip adds manually (I really do despise the 30 mins adds when I put youtube on just for background noise).  


LeAnn Alexandra 2024