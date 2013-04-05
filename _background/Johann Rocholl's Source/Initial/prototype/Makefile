BZR_HOST = johann.rocholl.net
BZR_PATH = /home/johann/checkout/prototype/
TYPES = -T AppData -T Bar -T Change -T Peak -T QuietZone -T Simulator \
-T CvCapture -T CvFont -T CvPoint -T IplImage -T FILE -T uchar

build :
	$(MAKE) -C build

indent :
	gnuindent -npro -kr -i4 -ts4 $(TYPES) *.c decoder/*.{h,c}

bzr_update : bzr_push
	ssh $(BZR_HOST) "cd $(BZR_PATH) ; bzr update ; ./rss.sh > rss.xml"

bzr_push :
	bzr push bzr+ssh://$(BZR_HOST)$(BZR_PATH)

bzr_pull :
	bzr pull bzr+ssh://$(BZR_HOST)$(BZR_PATH)

gnuplot : gnuplot/*.pdf

gnuplot/%.pdf : gnuplot/%.plt gnuplot/%.dat
	gnuplot $<

.PHONY : build bzr_update bzr_push bzr_pull
