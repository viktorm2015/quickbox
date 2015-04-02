import QtQml 2.0
import qf.core 1.0
import qf.qmlwidgets 1.0
import CardReader 1.0

CardChecker 
{
	id: root
	caption: "Classic"
	
	function checkCard(card_in, run_id)
	{
		Log.info("checking card:", JSON.stringify(card_in, null, 2));

		var course = root.courseForRunId(run_id);
		Log.info("course:", JSON.stringify(course, null, 2));
		
		var card_out = {courseId: course.id, punchList: []};
		var punches_in = card_in.punchList;
		var punches_out = card_out.punchList;
		for(var k=0; k<punches_in.length; k++) {
			var punch_in = punches_in[k];
			punches_out.push({code: punch_in.code});
		}

		var course_codes = course.codes;
		var error = false;
		var check_ix = 0;
		for(var j=0; j<course_codes.length; j++) { //scan course codes
			var course_code_record = course_codes[j];

			var code = course_code_record.code;
			for(var k=check_ix; k<punches_out.length; k++) { //scan card
				//var punch_in = punches_in[k];
				var punch_out = punches_out[k];
				if(punch_out.code == code) {  
					punch_out.position = course_code_record.position;
					check_ix = k + 1;
					break;
				}
			}
			if(k >= punches_out.length) {// course code not found in card_in punches
				var nocheck = course_code_record.outoforder;
				if(!nocheck) 
					error = true;
			}
		}
		if(card_in.finishTime == 0xEEEE) 
			error = true;
		card_out.isOk = !error

		//........... normalize times .....................
		card_out.startTimeMs = 0;
		if(card_in.startTime == 0xEEEE)        //take start record from start list
			card_out.startTimeMs = root.stageStartSec() + root.startTimeSec(run_id);
		else 
			card_out.startTimeMs = card_in.startTime;
		
		// set start time to be AM even if it is night race, SI cards have 12 hrs wrap-around
		card_out.startTimeMs = root.toAMms(card_out.startTimeMs * 1000);

		card_out.finishTimeMs = 0;
		//card_out.lapTimeMs = 0;
		if(card_in.finishTime != 0xEEEE) {
			card_out.finishTimeMs = root.fixTimeWrap(card_out.startTimeMs, 1000 * card_in.finishTime + card_in.finishTimeMs);
			//card_out.lapTimeMs = card_out.finishTimeMs - card_out.startTimeMs;
		}

		var prev_position = 0;
		var prev_position_stp = card_out.startTimeMs;
		for(var k=0; k<punches_in.length; k++) { //compute lap times
			var punch_in = punches_in[k];
			var punch_out = punches_out[k];
			punch_out.stpTimeMs = root.toAMms(punch_in.time * 1000 + punch_in.msec - card_out.startTimeMs);
			punch_out.lapTimeMs = 0;
			if(punch_out.position > prev_position) {  // positions are starting with 1, like 1,2,3,4,5
				if(punch_out.position - 1 == prev_position) {  
					punch_out.lapTimeMs = punch_out.stpTimeMs - prev_position_stp;
				}
				prev_position = punch_out.position;
				prev_position_stp = punch_out.stpTimeMs;
			}
		}
		Log.info("check result:", JSON.stringify(card_out, null, 2));
		return card_out;
	}
}