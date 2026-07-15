#include "DxLib.h"
#include <stdlib.h>
#include <math.h>

// バイクの画像を管理する定数と配列
const int BIKE_MAX = 2;
int imgBike[BIKE_MAX];

// 破片エフェクト用の構造体定義
struct Debris {
	float x;
	float y;
	float vx;
	float vy;
	int timer;     // 消滅までのタイマー
	bool isActive; // 出現中フラグ
};

// 排気ガス（煙）エフェクト用の構造体
struct Smoke 
{
	float x;
	float y;
	float vx;
	float vy;
	float size;      // 煙の大きさ
	int alpha;       // 煙の透明度
	bool isActive;   // 出現中フラグ
};

// バイクの表示位置と判定四角形の更新
void DrawBikeAndHitBox(int x, int y, int type, int* rL, int* rR, int* rT, int* rB, int* fL, int* fR, int* fT, int* fB)
{
	*rL = x + 5;       // 後輪の左端
	*rR = x + 95;      // 後輪の右端
	*rT = y + 125;     // タイヤのゴム部分
	*rB = y + 155;     // タイヤの接地している一番底辺

	*fL = x + 190;     // 前輪の左端
	*fR = x + 290;     // 前輪の右端
	*fT = y + 125;     // タイヤのゴム部分
	*fB = y + 155;     // タイヤの接地している一番底辺
}

// 2つの円が当たっているか計算する関数
bool CheckCircleHit(int c1x, int c1y, int c1r, int c2x, int c2y, int c2r)
{
	int dx = c1x - c2x;
	int dy = c1y - c2y;
	int distanceSquare = dx * dx + dy * dy;
	int radiusSum = c1r + c2r;
	return distanceSquare < (radiusSum * radiusSum);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const int WIDTH = 1014, HEIGHT = 768;

	SetWindowText("バイクゲーム");
	SetGraphMode(WIDTH, HEIGHT, 32);
	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1) return -1;
	SetBackgroundColor(0, 0, 0);
	SetDrawScreen(DX_SCREEN_BACK);

	int bgX = 0;
	int imgBG = LoadGraph("Image/BG.jpg");

	// 2枚の画像ファイルを配列に読み込む
	imgBike[0] = LoadGraph("Image/Redbike0.png");
	imgBike[1] = LoadGraph("Image/Redbike1.png");

	// 岩の画像を読み込む
	int imgObstacle = LoadGraph("Image/Rock.png");
	const int OBSTACLE_WIDTH = 130, OBSTACLE_HEIGHT = 130;
	// 無敵アイテムの画像を読み込む
	int imgItem = LoadGraph("Image/Item1.png");
	const int ITEM_WIDTH = 50, ITEM_HEIGHT = 50;
	const int GHOST_MAX = 5;
	int ghostX[5] = { 0 };
	int ghostY[5] = { 0 };
	int ghostFrame[5] = { 0 };
	int ghostCount = 0;

	// スコアアップアイテムの画像を読み込む
	int imgScoreUP = LoadGraph("Image/ScoreUP.png");
	const int SCOREUP_WIDTH = 50, SCOREUP_HEIGHT = 50;
	// BGMを読み込む
	int soundBGM = LoadSoundMem("Sound/BGM.mp3");
	PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
	// バイクが壊れた時のSEを読み込む
	int soundHit = LoadSoundMem("Sound/BikeSE.mp3");
	// 無敵アイテムを取った時のSEを読み込む
	int soundItem = LoadSoundMem("Sound/Item1SE.mp3");
	// スコアアイテムを取った時のSEを読み込む
	int soundScoreUP = LoadSoundMem("Sound/ScoreUPSE.mp3");
	// 岩を破壊した時のSEを読み込む
	int soundDestroy = LoadSoundMem("Sound/DestroySE.mp3");
	// 無敵中のSEを読み込む
	int soundInvincible = LoadSoundMem("Sound/InvincibleSE.mp3");

	const int FLOOR_Y = 472;

	int playerX = 100;
	int playerY = FLOOR_Y;
	int playerVY = 0;

	int obstacleX = WIDTH;
	int obstacleY = FLOOR_Y + 20;
	int obstacleSpeed = 15;

	int itemX = WIDTH + 1500;
	int itemY = FLOOR_Y - 100;
	int itemSpeed = 15;
	int isItemExit = 1;

	int scoreUpX = WIDTH + 2000;
	int scoreUpY = FLOOR_Y - 80;
	int scoreUpSpeed = 15;
	int isScoreUpExit = 1;

	// 無敵タイマーとゲームオーバーフラグ
	int invincibleTimer = 0;
	int isGameOver = 0;
	int score = 0;
	int hiscore = 0;

	int gameState = 0;
	int comboCount = 0;

	// アニメーション制御用の変数
	int animeTimer = 0;
	int animeFrame = 0;

	int rearLeft = 0, rearRight = 0, rearTop = 0, rearBottom = 0;
	int frontLeft = 0, frontRight = 0, frontTop = 0, frontBottom = 0;

	// エフェクト用配列をwhileループの外側で宣言
	const int DEBRIS_MAX = 15;
	Debris debris[DEBRIS_MAX];
	for (int i = 0; i < DEBRIS_MAX; i++) 
	{
		debris[i].isActive = false;
	}

	// 煙用の配列と最大数
	const int SMOKE_MAX = 50;
	Smoke smoke[SMOKE_MAX];
	for (int i = 0; i < SMOKE_MAX; i++)
	{
		smoke[i].isActive = false;
	}

	float angle = 0.0f;

	// メインループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面をクリア
		ClearDrawScreen();

		// タイトル画面の処理
		if (gameState == 0)
		{
			bgX = bgX - 2;
			if (bgX <= -WIDTH) bgX = bgX + WIDTH;

			if (CheckHitKey(KEY_INPUT_SPACE) == 1)
			{
				playerX = 100; playerY = FLOOR_Y; playerVY = 0;
				obstacleX = WIDTH; obstacleY = FLOOR_Y + 20;
				itemX = WIDTH + rand() % 2000; itemY = FLOOR_Y - 100; isItemExit = 1;
				scoreUpX = WIDTH + rand() % 2000 + 1000; scoreUpY = FLOOR_Y - 80; isScoreUpExit = 1;
				invincibleTimer = 0; isGameOver = 0; score = 0; comboCount = 0; animeTimer = 0; animeFrame = 0;
				angle = 0.0f;

				// スタート時に破片をすべて消す
				for (int i = 0; i < DEBRIS_MAX; i++) debris[i].isActive = false;

				StopSoundMem(soundInvincible);
				PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
				gameState = 1;
			}
		}
		// ゲームプレイ中の処理
		else if (gameState == 1)
		{

			// 煙の移動＆フェードアウト処理
			for (int i = 0; i < SMOKE_MAX; i++)
			{
				if (smoke[i].isActive)
				{
					smoke[i].x += smoke[i].vx;
					smoke[i].y += smoke[i].vy;
					smoke[i].size += 0.3f;       // 煙が徐々に広がる
					smoke[i].alpha -= 8;         // 徐々に薄くなる

					if (smoke[i].alpha <= 0)
					{
						smoke[i].isActive = false;
					}
				}
			}

			// 毎フレーム、バイクの排気口（後輪のちょい後ろ）から煙を発生させる
			if (isGameOver == 0)
			{
				// 煙を出すタイミング
				if (animeTimer % 2 == 0)
				{
					for (int i = 0; i < SMOKE_MAX; i++)
					{
						if (!smoke[i].isActive)
						{
							smoke[i].isActive = true;
							// バイクの後輪付近を排気口に見立てる
							smoke[i].x = (float)(playerX + 15);
							smoke[i].y = (float)(playerY + 125);

							// 煙が後ろに流れるように速度を設定
							smoke[i].vx = (float)(-(rand() % 4 + 4)); // 左向きの風
							smoke[i].vy = (float)((rand() % 3 - 1) * 0.5f); // ほんの少し上下にブレる
							smoke[i].size = 5.0f; // 初期サイズ
							smoke[i].alpha = 180; // 初期の濃さ
							break; // 1フレームに1個出せば十分なのでループを抜ける！
						}
					}
				}
			}
			// 破片エフェクトの移動（位置更新）処理
			for (int i = 0; i < DEBRIS_MAX; i++)
			{
				if (debris[i].isActive)
				{
					debris[i].x += debris[i].vx;
					debris[i].vy += 0.6f; // 重力加速度
					debris[i].y += debris[i].vy;

					debris[i].timer--;
					if (debris[i].timer <= 0)
					{
						debris[i].isActive = false;
					}
				}
			}

			if (isGameOver == 0)
			{
				// 無敵タイマーの処理
				if (invincibleTimer > 0)
				{
					invincibleTimer--;
					if (invincibleTimer == 0)
					{
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE);
					}
				}

				// ゲームの更新処理 
				bgX = bgX - 2;
				if (bgX <= -WIDTH) bgX = bgX + WIDTH;

				// スペースキーでジャンプ
				if (CheckHitKey(KEY_INPUT_SPACE) == 1 && playerY == FLOOR_Y)
				{
					playerVY = -20;
				}

				// 重力の処理
				playerVY += 1;
				playerY += playerVY;

				if (playerY > FLOOR_Y)
				{
					playerY = FLOOR_Y;
					playerVY = 0;

					// パラパラ動くタイマー
					animeTimer++;
					if (animeTimer % 6 == 0)
					{
						animeFrame = (animeFrame + 1) % BIKE_MAX;
					}
				}
				else
				{
					animeFrame = 0;
				}

				// 障害物の移動
				obstacleX -= obstacleSpeed;
				if (obstacleX < -OBSTACLE_WIDTH)
				{
					obstacleX = WIDTH;
					obstacleY = FLOOR_Y + 20;
					score += 10;
					if (score > hiscore) hiscore = score;
				}

				// 無敵アイテムの移動
				itemX -= itemSpeed;
				if (itemX < -ITEM_WIDTH)
				{
					itemX = WIDTH + rand() % 2000 + rand() % 2500;
					itemY = FLOOR_Y - 100;
					isItemExit = 1;
				}

				// スコアアップアイテムの移動
				scoreUpX -= scoreUpSpeed;
				if (scoreUpX < -SCOREUP_WIDTH)
				{
					comboCount = 0;
					scoreUpX = WIDTH + rand() % 2000 + rand() % 2500;
					scoreUpY = FLOOR_Y - (60 + rand() % 60);
					isScoreUpExit = 1;

					if (abs(scoreUpX - itemX) < 400)
					{
						scoreUpX += 500;
					}
				}

				// プレイヤーの位置を残像配列に記録
				ghostX[ghostCount] = playerX;
				ghostY[ghostCount] = playerY;
				ghostFrame[ghostCount] = animeFrame;
				ghostCount = (ghostCount + 1) % GHOST_MAX;

				DrawBikeAndHitBox(playerX, playerY, animeFrame,
					&rearLeft, &rearRight, &rearTop, &rearBottom,
					&frontLeft, &frontRight, &frontTop, &frontBottom);

				// 障害物の中心座標と半径
				int oCenter_X = obstacleX + 80; int oCenter_Y = obstacleY + 95; int oCenter_R = 45;
				int oLeft_X = obstacleX + 35;   int oLeft_Y = obstacleY + 130;  int oLeft_R = 25;
				int oRight_X = obstacleX + 120; int oRight_Y = obstacleY + 130; int oRight_R = 25;

				int iCenter_X = itemX + (ITEM_WIDTH / 2);
				int iCenter_Y = itemY + (ITEM_HEIGHT / 2);
				int iCenter_R = 30;

				int rearCenterX = (rearLeft + rearRight) / 2;
				int rearCenterY = (rearTop + rearBottom) / 2;
				int rearRadius = (rearRight - rearLeft) / 2;

				int frontCenterX = (frontLeft + frontRight) / 2;
				int frontCenterY = (frontTop + frontBottom) / 2;
				int frontRadius = (frontRight - frontLeft) / 2;

				// 無敵アイテムの当たり判定
				if (isItemExit == 1)
				{
					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, iCenter_X, iCenter_Y, iCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, iCenter_X, iCenter_Y, iCenter_R))
					{
						isItemExit = 0;
						invincibleTimer = 180;
						score += 30;
						if (score > hiscore) hiscore = score;

						PlaySoundMem(soundItem, DX_PLAYTYPE_BACK, TRUE);
						StopSoundMem(soundBGM);
						PlaySoundMem(soundInvincible, DX_PLAYTYPE_BACK, TRUE);
					}
				}

				// スコアアップアイテムの当たり判定
				if (isScoreUpExit == 1)
				{
					int suCenter_X = scoreUpX + (SCOREUP_WIDTH / 2);
					int suCenter_Y = scoreUpY + (SCOREUP_HEIGHT / 2);
					int suCenter_R = 25;

					if (CheckCircleHit(rearCenterX, rearCenterY, rearRadius, suCenter_X, suCenter_Y, suCenter_R) ||
						CheckCircleHit(frontCenterX, frontCenterY, frontRadius, suCenter_X, suCenter_Y, suCenter_R))
					{
						isScoreUpExit = 0;
						comboCount++;
						score += 100 * comboCount;
						if (score > hiscore) hiscore = score;

						PlaySoundMem(soundScoreUP, DX_PLAYTYPE_BACK, TRUE);
					}
				}

				// 障害物の当たり判定
				bool isRearHit = CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(rearCenterX, rearCenterY, rearRadius, oRight_X, oRight_Y, oRight_R);

				bool isFrontHit = CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oCenter_X, oCenter_Y, oCenter_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oLeft_X, oLeft_Y, oLeft_R) ||
					CheckCircleHit(frontCenterX, frontCenterY, frontRadius, oRight_X, oRight_Y, oRight_R);

				if (isRearHit || isFrontHit)
				{
					if (invincibleTimer > 0)
					{
						int burstCenterX = obstacleX + 80;
						int burstCenterY = obstacleY + 95;

						// 破片データを生成
						for (int i = 0; i < DEBRIS_MAX; i++) {
							debris[i].x = (float)burstCenterX;
							debris[i].y = (float)burstCenterY;
							debris[i].vx = (float)((rand() % 25) - 12);
							debris[i].vy = (float)(-(rand() % 9) - 7);
							debris[i].timer = 45;
							debris[i].isActive = true;
						}

						obstacleX = WIDTH;
						obstacleY = FLOOR_Y + 20;

						score += 20;
						if (score > hiscore) hiscore = score;
						PlaySoundMem(soundDestroy, DX_PLAYTYPE_BACK, TRUE);
					}
					else
					{
						isGameOver = 1;
						StopSoundMem(soundBGM);
						StopSoundMem(soundInvincible);
						PlaySoundMem(soundHit, DX_PLAYTYPE_BACK, TRUE);
					}
				}
			}
			else
			{
				// ゲームオーバー時のキー待ち
				if (CheckHitKey(KEY_INPUT_R) == 1)
				{
					playerX = 100; playerY = FLOOR_Y; playerVY = 0; obstacleX = WIDTH; obstacleY = FLOOR_Y + 20; itemX = WIDTH + rand() % 2000; itemY = FLOOR_Y - 100; isItemExit = 1; scoreUpX = WIDTH + rand() % 2000 + 1000; scoreUpY = FLOOR_Y - 80; isScoreUpExit = 1; invincibleTimer = 0; isGameOver = 0; score = 0; comboCount = 0; animeTimer = 0; animeFrame = 0; angle = 0.0f; for (int i = 0; i < DEBRIS_MAX; i++) debris[i].isActive = false; StopSoundMem(soundInvincible); PlaySoundMem(soundBGM, DX_PLAYTYPE_LOOP, TRUE); DrawBikeAndHitBox(playerX, playerY, animeFrame, &rearLeft, &rearRight, &rearTop, &rearBottom, &frontLeft, &frontRight, &frontTop, &frontBottom);
				}
				if (CheckHitKey(KEY_INPUT_T) == 1)
				{
					gameState = 0;
				}
			}
		}

		// 描画セクション
		DrawGraph(bgX, 0, imgBG, FALSE);
		DrawGraph(bgX + WIDTH, 0, imgBG, FALSE);

		if (gameState == 0)
		{
			DrawString(WIDTH / 2 - 70, HEIGHT / 2 - 20, "BIKE GAME", GetColor(255, 255, 255));
			DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press Space Key to Start", GetColor(255, 255, 0));
		}
		else if (gameState == 1)

			// 煙の描画
			for (int i = 0; i < SMOKE_MAX; i++)
			{
				if (smoke[i].isActive)
				{
					// アルファブレンド（半透明）を有効にする
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, smoke[i].alpha);

					// 煙を円で描画（グレーっぽい色）
					DrawCircle((int)smoke[i].x, (int)smoke[i].y, (int)smoke[i].size, GetColor(180, 180, 180), TRUE);
				}
			}
		// ブレンドモードを通常に戻す
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		{
			// 生きているかどうかに左右されず、画面に破片を描画する
			for (int i = 0; i < DEBRIS_MAX; i++)
			{
				if (debris[i].isActive)
				{
					// 岩の画像を 20x20 のサイズに縮小して、それぞれの破片の座標へ描画
					DrawExtendGraph((int)debris[i].x, (int)debris[i].y, (int)debris[i].x + 20, (int)debris[i].y + 20, imgObstacle, TRUE);
				}
			}
			// 通常オブジェクト描画
			DrawExtendGraph(obstacleX, obstacleY + 20, obstacleX + OBSTACLE_WIDTH, obstacleY + OBSTACLE_HEIGHT + 20, imgObstacle, TRUE);
			if (isItemExit == 1)
				DrawExtendGraph(itemX, itemY, itemX + ITEM_WIDTH, itemY + ITEM_HEIGHT, imgItem, TRUE);
			if (isScoreUpExit == 1)
				DrawExtendGraph(scoreUpX, scoreUpY, scoreUpX + SCOREUP_WIDTH, scoreUpY + SCOREUP_HEIGHT, imgScoreUP, TRUE);

			// バイクの傾き
			if (playerY < FLOOR_Y)
			{
				angle = playerVY * 0.015f;
				if (angle < -0.45f) angle = -0.45f;
				if (angle > 0.45f) angle = 0.45f;
			}
			else
			{
				angle = 0.0f; // 地面では水平に固定
			}

			const int BIKE_HALF_W = 141;
			const int BIKE_HALF_H = 85;

			// 自機と無敵残像の描画
			if (invincibleTimer > 0)
			{
				for (int i = 0; i < GHOST_MAX; i++)
				{
					int idx = (ghostCount + i) % GHOST_MAX;
					int alpha = 50 + (i * 40);
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
					int offset_x = ghostX[idx] - (GHOST_MAX - i) * 6;
					DrawRotaGraph(offset_x + BIKE_HALF_W, ghostY[idx] + BIKE_HALF_H, 1.0, angle, imgBike[ghostFrame[idx]], TRUE);
				}
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
				if ((invincibleTimer / 4) % 2 == 0)
				{
					DrawRotaGraph(playerX + BIKE_HALF_W, playerY + BIKE_HALF_H, 1.0, angle, imgBike[animeFrame], TRUE);
				}
			}
			else
			{
				DrawRotaGraph(playerX + BIKE_HALF_W, playerY + BIKE_HALF_H, 1.0, angle, imgBike[animeFrame], TRUE);
			}

			// UIスコア描画
			DrawFormatString(20, 20, GetColor(255, 255, 255), "SCORE: %d", score);
			DrawFormatString(20, 50, GetColor(255, 255, 0), "HISCORE: %d", hiscore);
			if (comboCount >= 2)
			{
				SetFontSize(40);
				DrawFormatString(WIDTH / 2 - 80, 100, GetColor(255, 200, 0), "%d COMBO!", comboCount);
				SetFontSize(16);
			}
			if (invincibleTimer > 0)
			{
				DrawBox(20, 80, 20 + invincibleTimer, 95, GetColor(255, 128, 0), TRUE);
				DrawString(25, 80, "INVINCIBLE!", GetColor(255, 255, 255));
			}
			if (isGameOver == 1)
			{
				DrawString(WIDTH / 2 - 100, HEIGHT / 2 - 20, "GAME OVER", GetColor(255, 0, 0));
				DrawString(WIDTH / 2 - 140, HEIGHT / 2 + 10, "Press R Key to Restart / T Key to Title", GetColor(255, 255, 255));
			}
		}

		ScreenFlip();
		WaitTimer(16);
	}

	DxLib_End();
	return 0;
}