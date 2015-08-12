#pragma once

struct CameraArea;
struct CameraView;

class ICameraService
{
public:
	virtual const ff::RectFixedInt& GetCameraBounds() = 0;
	virtual const ff::Vector<CameraArea>& GetCameraAreas() = 0;
	virtual const CameraView& GetAdvanceCameraView() = 0;
	virtual const CameraView& GetRenderCameraView() = 0;
	virtual const CameraView& GetFinalCameraView() = 0;
};
