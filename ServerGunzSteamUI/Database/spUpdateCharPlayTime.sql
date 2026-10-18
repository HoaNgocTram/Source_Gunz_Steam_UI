ALTER PROC [dbo].[spUpdateCharPlayTime]
  @PlayTimeInc    int,
  @AID            int
AS 
SET NOCOUNT ON

-- Cập nhật tổng thời gian chung vào bảng Character như cũ
UPDATE Character 
SET PlayTime = PlayTime + @PlayTimeInc, 
    LastTime = GETDATE() 
WHERE AID  = @AID ;

-- Kiểm tra và cập nhật thời gian sự kiện theo ngày trong bảng PlayTimeEvent
IF EXISTS (SELECT 1 FROM PlayTimeEvent WHERE AID  = @AID )
BEGIN
    UPDATE PlayTimeEvent 
    SET AccumulatedMinutes = CASE 
            WHEN DATEDIFF(day, LastUpdated, GETDATE()) > 0 THEN @PlayTimeInc -- Sang ngày mới thì tính lại từ đầu
            ELSE AccumulatedMinutes + @PlayTimeInc                        -- Cùng ngày thì cộng dồn
         END,
        NormalStateMask = CASE 
            WHEN DATEDIFF(day, LastUpdated, GETDATE()) > 0 THEN 0          -- Reset trạng thái quà Normal ngày mới
            ELSE NormalStateMask 
         END,
        PlusStateMask = CASE 
            WHEN DATEDIFF(day, LastUpdated, GETDATE()) > 0 THEN 0          -- Reset trạng thái quà Plus ngày mới
            ELSE PlusStateMask 
         END,
        LastUpdated = GETDATE()
    WHERE AID  = @AID ;
END
ELSE
BEGIN
    -- Nếu nhân vật chưa có bản ghi thì khởi tạo mới
    INSERT INTO PlayTimeEvent (AID , AccumulatedMinutes, NormalStateMask, PlusStateMask, LastUpdated)
    VALUES (@AID , @PlayTimeInc, 0, 0, GETDATE());
END