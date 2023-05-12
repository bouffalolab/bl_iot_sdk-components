#ifndef __BL808_ISP_WDR_H__
#define __BL808_ISP_WDR_H__

#include <bl808_isp.h>

#define ISP_WDR_HIST_ROI_NUM                (8)
#define ISP_WDR_HIST_BIN_NUM                (65)
#define ISP_WDR_CURVE_COEF_NUM              (64)

typedef struct {
    uint32_t hist_bin[ISP_WDR_HIST_BIN_NUM];
} ISP_WDR_Hist_Data_Type;

typedef struct {
    uint16_t curve_coef[ISP_WDR_CURVE_COEF_NUM];
} ISP_WDR_Curve_Type;

void ISP_WDR_Set_WDR_State(BL_Fun_Type state);
void ISP_WDR_Get_WDR_State(BL_Fun_Type *state);
void ISP_WDR_Set_CS_State(BL_Fun_Type state);
void ISP_WDR_Get_CS_State(BL_Fun_Type *state);
void ISP_WDR_Set_CS_Config(const ISP_Chroma_Suppress_Type *chromaSupprCfg);
void ISP_WDR_Get_CS_Config(ISP_Chroma_Suppress_Type *chromaSupprCfg);
void ISP_WDR_Set_ROI_State(int roi_id, BL_Fun_Type state);
void ISP_WDR_Get_ROI_State(int roi_id, BL_Fun_Type *state);
void ISP_WDR_Set_ROI_Position(int roi_id, int left_x, int right_x,
                                int top_y, int bottom_y);
void ISP_WDR_Get_ROI_Position(int roi_id, int *left_x, int *right_x,
                                int *top_y, int *bottom_y);
void ISP_WDR_Set_ROI_Weight(int roi_id, int weight);
void ISP_WDR_Get_ROI_Weight(int roi_id, int *weight);
int  ISP_WDR_Get_Hist_Data(ISP_WDR_Hist_Data_Type *hist);
void ISP_WDR_Set_WDR_Curve_State(BL_Fun_Type state);
void ISP_WDR_Get_WDR_Curve_State(BL_Fun_Type *state);
void ISP_WDR_Set_WDR_Curve(const ISP_WDR_Curve_Type *wdr_curve);
void ISP_WDR_Get_WDR_Curve(ISP_WDR_Curve_Type *wdr_curve);
void ISP_WDR_Set_WDR_Curve_Weight(int weight);
void ISP_WDR_Get_WDR_Curve_Weight(int *weight);
void ISP_WDR_Set_Post_Enhance_State(BL_Fun_Type state);
void ISP_WDR_Get_Post_Enhance_State(BL_Fun_Type *state);
void ISP_WDR_Set_Post_Enhance_Weight(int weight);
void ISP_WDR_Get_Post_Enhance_Weight(int *weight);
void ISP_WDR_Set_Post_Enhance_Y_Offset(int y_offset);
void ISP_WDR_Get_Post_Enhance_Y_Offset(int *y_offset);
void ISP_WDR_Set_Post_Enhance_Luma_Gain(int gain);
void ISP_WDR_Get_Post_Enhance_Luma_Gain(int *gain);
void ISP_WDR_Set_Post_Enhance_Chroma_Gain(int gain);
void ISP_WDR_Get_Post_Enhance_Chroma_Gain(int *gain);

#endif /* __BL808_WDR_H__ */
