#include <Graphics.h>
#include <TextField.h>
#include <Display.h>

RenderState gState;

Graphics  gGraphics;
TextField gText;

void Handler(Event &ioEvent,void *inStage)
{
	Stage *stage = (Stage *)inStage;

	static int tx = 0;
	static float rot = 0;
	tx = (tx+1) % 500;
	rot += 1;
	if (rot>360) rot-=360;

	if (ioEvent.mType==etNextFrame)
	{
		AutoStageRender render(stage,0xffffff);
		gState.mTransform.mMatrix = Matrix().Rotate(rot).Translate(tx+100,200);
		gState.mClipRect = Rect( render.Width(), render.Height() );
		gState.mTransform.mAAFactor = 4;
		gState.mAAClipRect = gState.mClipRect * gState.mTransform.mAAFactor;

		gGraphics.Render(render.Target(),gState);
		gState.mTransform.mMatrix = Matrix();
		gText.Render(render.Target(),gState);
	}
}


int main(int inargc,char **arvg)
{
   Frame *frame = CreateMainFrame(640,400,wfResizable,L"Hello");


	// Render to bitmap ...
	Surface *bg = new SimpleSurface(32,32, pfXRGB);
	{
	   Graphics gfx;
	   gfx.lineStyle(1,0x0000ff);
	   gfx.moveTo(0,0);
	   gfx.lineTo(32,32);
	   gfx.moveTo(0,32);
	   gfx.lineTo(32,0);
		bg->Clear(0x002020);
		AutoSurfaceRender render(bg);
		gfx.Render(render.Target(),RenderState(bg,4));
	}



	frame->GetStage()->SetEventHandler(Handler,frame->GetStage());

	GraphicsGradientFill *fill = new GraphicsGradientFill(true,
											Matrix().createGradientBox(200,200,45,-100,-100), smPad );
	fill->AddStop( 0xffffff, 1, 0 );
	fill->AddStop( 0xff0000, 1, 0.25 );
	fill->AddStop( 0x00ff00, 1, 0.5 );
	fill->AddStop( 0x0000ff, 1, 0.75 );
	fill->AddStop( 0xff00ff, 1, 1 );
	gGraphics.addData(fill);

	//gGraphics.beginBitmapFill(bg, Matrix().createGradientBox(32,32), true,true );

	gGraphics.lineStyle(5,0x202040,0.75,true,ssmNormal,scRound,sjMiter);
	gGraphics.moveTo(-100,-100);
	gGraphics.lineTo(-100,100);
	gGraphics.curveTo(0,180,100,100);
	gGraphics.lineTo(100,-100);
	gGraphics.lineTo(-100,-100);

	Extent2DF ext = gGraphics.GetExtent(gState.mTransform);
	printf("Extent %f,%f ... %f,%f\n", ext.mMinX, ext.mMinY, ext.mMaxX, ext.mMaxY);

	//gText.setText(L"Hello, abcdefghijklmnopqrstuvwxyz 1234567890 ABCDEFGHIGKLMNOPQRSTUVWXYZjjj");
	//gText.setHTMLText(L"HHHH");
	gText.mRect.x = 200;
	gText.mRect.y = 2;
	gText.background = true;
	gText.backgroundColor.SetRGBNative(0xb0b0f0);
	gText.autoSize = asLeft;
	gText.multiline = true;
	gText.wordWrap = true;
	gText.mRect.w = 50;
	gText.embedFonts = true;
	gText.setHTMLText(L"<font size=20>Hello <font color='#202060' face='times'>go\nod-<br>bye <b>gone for good!</b></font></font>");
	//gText.setHTMLText(L"H");

   MainLoop();
   delete frame;
   return 0;
}
